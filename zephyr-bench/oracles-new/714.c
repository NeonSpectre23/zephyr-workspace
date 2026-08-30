ssize_t settings_load_one(const char *name, void *buf, size_t buf_len)
{
	struct settings_store *cs;
	size_t val_len = 0;
	int rc = 0;

	/*
	 * For every config store that defines csi_load_one() function use it.
	 * Otherwise, use the csi_load() function to load the key/value pair
	 */
	settings_lock_take();
	SYS_SLIST_FOR_EACH_CONTAINER(&settings_load_srcs, cs, cs_next) {
		if (cs->cs_itf->csi_load_one) {
			rc = cs->cs_itf->csi_load_one(cs, name, (char *)buf, buf_len);
			val_len = (rc >= 0) ? rc : 0;
		} else {
			struct default_param param = {
				.buf = buf,
				.buf_len = buf_len,
				.val_len = &val_len
			};
			const struct settings_load_arg arg = {
				.subtree = name,
				.cb = &settings_set_default_cb,
				.param = &param
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