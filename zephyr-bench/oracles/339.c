int settings_save_one(const char *name, const void *value, size_t val_len)
{
	int rc;
	struct settings_store *cs;

	cs = settings_save_dst;
	if (!cs) {
		return -ENOENT;
	}

	settings_lock_take();

	rc = cs->cs_itf->csi_save(cs, name, (char *)value, val_len);

	settings_lock_release();

	return rc;
}