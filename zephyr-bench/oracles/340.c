int settings_storage_get(void **storage)
{
	struct settings_store *cs = settings_save_dst;

	if (!cs) {
		return -ENOENT;
	}

	if (cs->cs_itf->csi_storage_get) {
		*storage = cs->cs_itf->csi_storage_get(cs);
	}

	return 0;
}