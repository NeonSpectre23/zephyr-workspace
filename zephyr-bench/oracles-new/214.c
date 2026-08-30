void k_mem_map_phys_bare(uint8_t **virt_ptr, uintptr_t phys, size_t size, uint32_t flags)
{
	uintptr_t aligned_phys, addr_offset;
	size_t aligned_size, align_boundary;
	k_spinlock_key_t key;
	uint8_t *dest_addr;
	size_t num_bits;
	size_t offset;

#ifndef CONFIG_KERNEL_DIRECT_MAP
	__ASSERT(!(flags & K_MEM_DIRECT_MAP), "The direct-map is not enabled");
#endif /* CONFIG_KERNEL_DIRECT_MAP */
	addr_offset = k_mem_region_align(&aligned_phys, &aligned_size,
					 phys, size,
					 CONFIG_MMU_PAGE_SIZE);
	__ASSERT(aligned_size != 0U, "0-length mapping at 0x%lx", aligned_phys);
	__ASSERT(aligned_phys < (aligned_phys + (aligned_size - 1)),
		 "wraparound for physical address 0x%lx (size %zu)",
		 aligned_phys, aligned_size);

	align_boundary = arch_virt_region_align(aligned_phys, aligned_size);

	key = k_spin_lock(&z_mm_lock);

	if (IS_ENABLED(CONFIG_KERNEL_DIRECT_MAP) &&
	    (flags & K_MEM_DIRECT_MAP)) {
		dest_addr = (uint8_t *)aligned_phys;

		/* Mark the region of virtual memory bitmap as used
		 * if the region overlaps the virtual memory space.
		 *
		 * Basically if either end of region is within
		 * virtual memory space, we need to mark the bits.
		 */

		if (IN_RANGE(aligned_phys,
			      (uintptr_t)K_MEM_VIRT_RAM_START,
			      (uintptr_t)(K_MEM_VIRT_RAM_END - 1)) ||
		    IN_RANGE(aligned_phys + aligned_size - 1,
			      (uintptr_t)K_MEM_VIRT_RAM_START,
			      (uintptr_t)(K_MEM_VIRT_RAM_END - 1))) {
			uint8_t *adjusted_start = max(dest_addr, K_MEM_VIRT_RAM_START);
			uint8_t *adjusted_end = min(dest_addr + aligned_size,
						    K_MEM_VIRT_RAM_END);
			size_t adjusted_sz = adjusted_end - adjusted_start;

			num_bits = adjusted_sz / CONFIG_MMU_PAGE_SIZE;
			offset = virt_to_bitmap_offset(adjusted_start, adjusted_sz);
			if (sys_bitarray_test_and_set_region(
			    &virt_region_bitmap, num_bits, offset, true)) {
				goto fail;
			}
		}
	} else {
		/* Obtain an appropriately sized chunk of virtual memory */
		dest_addr = virt_region_alloc(aligned_size, align_boundary);
		if (!dest_addr) {
			goto fail;
		}
	}

	/* If this fails there's something amiss with virt_region_get */
	__ASSERT((uintptr_t)dest_addr <
		 ((uintptr_t)dest_addr + (size - 1)),
		 "wraparound for virtual address %p (size %zu)",
		 dest_addr, size);

	LOG_DBG("arch_mem_map(%p, 0x%lx, %zu, %x) offset %lu", (void *)dest_addr,
		aligned_phys, aligned_size, flags, addr_offset);

	arch_mem_map(dest_addr, aligned_phys, aligned_size, flags);
	k_spin_unlock(&z_mm_lock, key);

	*virt_ptr = dest_addr + addr_offset;
	return;
fail:
	/* May re-visit this in the future, but for now running out of
	 * virtual address space or failing the arch_mem_map() call is
	 * an unrecoverable situation.
	 *
	 * Other problems not related to resource exhaustion we leave as
	 * assertions since they are clearly programming mistakes.
	 */
	LOG_ERR("memory mapping 0x%lx (size %zu, flags 0x%x) failed",
		phys, size, flags);
	k_panic();
}