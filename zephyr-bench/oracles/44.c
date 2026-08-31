int fcb_offset_last_n(struct fcb *fcbp, uint8_t entries, struct fcb_entry *last_n_entry)
{
	struct fcb_entry loc;
	int i;
	int rc;

	/* assure a minimum amount of entries */
	if (!entries) {
		entries = 1U;
	}

	i = 0;
	(void)memset(&loc, 0, sizeof(loc));
	while (!fcb_getnext(fcbp, &loc)) {
		if (i == 0) {
			/* Start from the beginning of fcb entries */
			*last_n_entry = loc;
		}
		/* Update last_n_entry after n entries and keep updating */
		else if (i > (entries - 1)) {
			rc = fcb_getnext(fcbp, last_n_entry);

			if (rc) {
				/* A fcb history must have been erased,
				 * wanted entry doesn't exist anymore.
				 */
				return -ENOENT;
			}
		}
		i++;
	}

	return (i == 0) ? -ENOENT : 0;
}