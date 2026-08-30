ssize_t zms_read(struct zms_fs *fs, zms_id_t id, void *data, size_t len)
{
	int rc;

	rc = zms_read_hist(fs, id, data, len, 0);
	if (rc < 0) {
		return rc;
	}

	/* returns the minimum between ATE data length and requested length */
	return MIN(rc, len);
}