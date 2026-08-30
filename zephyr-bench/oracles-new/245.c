int llext_section_shndx(const struct llext_loader *ldr, const struct llext *ext,
			const char *sect_name)
{
	unsigned int i;

	for (i = 1; i < ext->sect_cnt; i++) {
		const char *name = llext_section_name(ldr, ext, ext->sect_hdrs + i);

		if (!strcmp(name, sect_name)) {
			return i;
		}
	}

	return -ENOENT;
}