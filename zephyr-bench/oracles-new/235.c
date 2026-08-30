void llext_bootstrap(struct llext *ext, llext_entry_fn_t entry_fn, void *user_data)
{
	int ret;

	/* Call initialization functions */
	ret = llext_bringup(ext);
	if (ret < 0) {
		LOG_ERR("Failed to call init functions: %d", ret);
		return;
	}

	/* Start extension main function */
	LOG_DBG("calling entry function %p(%p)", (void *)entry_fn, user_data);
	entry_fn(user_data);

	/* Call de-initialization functions */
	ret = llext_teardown(ext);
	if (ret < 0) {
		LOG_ERR("Failed to call de-init functions: %d", ret);
		return;
	}
}