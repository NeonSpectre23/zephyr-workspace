ssize_t settings_get_val_len(const char *name)
{
	struct settings_store *cs;
	int rc = 0;
	size_t val_len = 0;

	/*
	 * for every config store that supports this function
	 * get the value's length.
	 */
	settings_lock_take();
	SYS_SLIST_FOR_EACH_CONTAINER(&settings_load_srcs, cs, cs_next) {
		if (cs->cs_itf->csi_get_val_len) {
			val_len = cs->cs_itf->csi_get_val_len(cs, name);
		} else {
			const struct settings_load_arg arg = {
				.subtree = name,
				.cb = &settings_get_val_len_default_cb,
				.param = &val_len
			};
			rc = cs->cs_itf->csi_load(cs, &arg);
		}
	}
	settings_lock_release();

	if (rc >= 0) {
		return val_len;
	}

	return rc;
}