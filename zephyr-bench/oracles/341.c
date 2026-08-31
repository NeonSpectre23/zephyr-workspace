int settings_subsys_init(void)
{

	int err = 0;

	settings_lock_take();

	if (!settings_subsys_initialized) {
		settings_init();

		err = settings_backend_init();

		if (!err) {
			settings_subsys_initialized = true;
		}
	}

	settings_lock_release();

	return err;
}