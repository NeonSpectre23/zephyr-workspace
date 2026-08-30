static int init(void)
{

	int ret;

	k_mutex_lock(&wifi_credentials_mutex, K_FOREVER);

	ret = wifi_credentials_backend_init();
	if (ret) {
		LOG_ERR("Initializing WiFi credentials storage backend failed, err: %d", ret);
	}

	k_mutex_unlock(&wifi_credentials_mutex);

	return 0;
}