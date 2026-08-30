static inline int llext_get_section_info(const struct llext_loader *ldr,
					 const struct llext *ext,
					 unsigned int shndx,
					 const elf_shdr_t **hdr,
					 enum llext_mem *region,
					 size_t *offset)
{
	if (shndx < 0 || shndx >= ext->sect_cnt) {
		return -EINVAL;
	}
	if (!ldr->sect_map) {
		return -ENOTSUP;
	}

	enum llext_mem mem_idx = ldr->sect_map[shndx].mem_idx;

	if (hdr) {
		*hdr = &ext->sect_hdrs[shndx];
	}
	if (region) {
		*region = mem_idx;
	}

	/* offset compensated for alignment prepad */
	if (offset) {
		*offset = ldr->sect_map[shndx].offset - ldr->sects[mem_idx].sh_info;
	}

	return 0;
}