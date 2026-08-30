static inline int llext_get_region_info(const struct llext_loader *ldr,
					const struct llext *ext,
					enum llext_mem region,
					const elf_shdr_t **hdr,
					const void **addr, size_t *size)
{
	if ((unsigned int)region >= LLEXT_MEM_COUNT) {
		return -EINVAL;
	}

	if (hdr) {
		*hdr = &ldr->sects[region];
	}

	/* address and size compensated for alignment prepad */
	if (addr) {
		*addr = (void *)((uintptr_t)ext->mem[region] + ldr->sects[region].sh_info);
	}
	if (size) {
		*size = ext->mem_size[region] - ldr->sects[region].sh_info;
	}

	return 0;
}