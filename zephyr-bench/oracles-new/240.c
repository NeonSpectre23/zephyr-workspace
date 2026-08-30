int llext_get_section_header(const struct llext_loader *ldr, const struct llext *ext,
			     const char *search_name, elf_shdr_t *shdr)
{
	int ret;

	ret = llext_section_shndx(ldr, ext, search_name);
	if (ret < 0) {
		return ret;
	}

	*shdr = ext->sect_hdrs[ret];
	return 0;
}