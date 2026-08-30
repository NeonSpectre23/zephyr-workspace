int mem_attr_check_buf(void *v_addr, size_t size, uint32_t attr)
{
	uintptr_t addr = (uintptr_t) v_addr;

	/*
	 * If MMU is enabled the address of the buffer is a virtual address
	 * while the addresses in the DT are physical addresses. Given that we
	 * have no way of knowing whether a mapping exists, we simply bail out.
	 */
	if (IS_ENABLED(CONFIG_MMU)) {
		return -ENOSYS;
	}

	if (size == 0) {
		return -ENOTSUP;
	}

	for (size_t idx = 0; idx < ARRAY_SIZE(mem_attr_region); idx++) {
		const struct mem_attr_region_t *region = &mem_attr_region[idx];
		size_t region_end = region->dt_addr + region->dt_size;

		/* Check if the buffer is in the region */
		if ((addr >= region->dt_addr) && (addr < region_end)) {
			/* Check if the buffer is entirely contained in the region */
			if ((addr + size) <= region_end) {
				/* check if the attribute is correct */
				return (region->dt_attr & attr) == attr ? 0 : -EINVAL;
			}
			return -ENOSPC;
		}
	}
	return -ENOBUFS;
}