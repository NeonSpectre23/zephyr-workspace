union log_msg_generic *z_log_msg_claim(k_timeout_t *backoff)
{
	size_t len;

	STRUCT_SECTION_COUNT(log_mpsc_pbuf, &len);

	/* Use only one buffer if others are not registered. */
	if (IS_ENABLED(CONFIG_LOG_MULTIDOMAIN) && len > 1) {
		return z_log_msg_claim_oldest(backoff);
	}

	return z_log_msg_local_claim();
}