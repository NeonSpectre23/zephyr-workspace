ssize_t zms_get_data_length(struct zms_fs *fs, zms_id_t id)
{
	int rc;

	rc = zms_read_hist(fs, id, NULL, 0, 0);

	return rc;
}