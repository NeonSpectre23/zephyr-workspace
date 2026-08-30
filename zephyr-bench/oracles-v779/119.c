uint32_t log_frontend_filter_get(int16_t source_id, bool runtime)
{
	if (!IS_ENABLED(CONFIG_LOG_FRONTEND)) {
		return LOG_LEVEL_NONE;
	}

	return filter_get(LOG_FRONTEND_SLOT_ID, Z_LOG_LOCAL_DOMAIN_ID, source_id, runtime);
}