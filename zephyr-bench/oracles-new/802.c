int
stats_walk(struct stats_hdr *hdr, stats_walk_fn *walk_func, void *arg)
{
	const char *name;
	char name_buf[STATS_GEN_NAME_MAX_LEN];
	int rc;
	int i;

	for (i = 0; i < hdr->s_cnt; i++) {
		name = stats_get_name(hdr, i);
		if (name == NULL) {
			/* No assigned name; generate a temporary s<#> name. */
			stats_gen_name(i, name_buf);
			name = name_buf;
		}

		rc = walk_func(hdr, arg, name, stats_get_off(hdr, i));
		if (rc != 0) {
			return rc;
		}
	}

	return 0;
}