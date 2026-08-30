int fs_unregister(int type, const struct fs_file_system_t *fs)
{
	int rc = 0;
	struct registry_entry *ep;

	k_mutex_lock(&mutex, K_FOREVER);

	ep = registry_find(type);
	if ((ep == NULL) || (ep->fstp != fs)) {
		rc = -EINVAL;
	} else {
		registry_clear_entry(ep);
	}

	k_mutex_unlock(&mutex);

	LOG_DBG("fs unregister %d: %d", type, rc);
	return rc;
}