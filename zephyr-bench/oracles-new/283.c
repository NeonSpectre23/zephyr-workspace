int log_source_id_get(const char *name)
{
	if (IS_ENABLED(CONFIG_LOG_FMT_SECTION_STRIP)) {
		return -1;
	}

	for (int i = 0; i < log_src_cnt_get(Z_LOG_LOCAL_DOMAIN_ID); i++) {
		const char *sname = log_source_name_get(Z_LOG_LOCAL_DOMAIN_ID, i);

		if ((sname != NULL) && (strcmp(sname, name) == 0)) {
			return i;
		}
	}
	return -1;
}