struct stats_hdr *
stats_group_find(const char *name)
{
	struct stats_hdr *hdr;

	for (hdr = stats_list; hdr != NULL; hdr = hdr->s_next) {
		if (strcmp(hdr->s_name, name) == 0) {
			return hdr;
		}
	}

	return NULL;
}