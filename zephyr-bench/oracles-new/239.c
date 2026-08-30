int llext_free_inspection_data(struct llext_loader *ldr, struct llext *ext)
{
	if (ldr->sect_map) {
		ext->alloc_size -= ext->sect_cnt * sizeof(ldr->sect_map[0]);
		llext_free_metadata(ldr->sect_map);
		ldr->sect_map = NULL;
	}

	return 0;
}