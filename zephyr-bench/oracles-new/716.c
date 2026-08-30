int settings_load_subtree_direct(
	const char             *subtree,
	settings_load_direct_cb cb,
	void                   *param)
{
	struct settings_store *cs;

	const struct settings_load_arg arg = {
		.subtree = subtree,
		.cb = cb,
		.param = param
	};
	/*
	 * for every config store
	 *    load config
	 *    apply config
	 *    commit all
	 */
	settings_lock_take();
	SYS_SLIST_FOR_EACH_CONTAINER(&settings_load_srcs, cs, cs_next) {
		cs->cs_itf->csi_load(cs, &arg);
	}
	settings_lock_release();
	return 0;
}