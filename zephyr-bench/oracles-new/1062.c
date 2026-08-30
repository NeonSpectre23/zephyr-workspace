int zms_delete(struct zms_fs *fs, zms_id_t id)
{
	return zms_write(fs, id, NULL, 0);
}