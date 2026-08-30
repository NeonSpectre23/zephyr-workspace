int nvs_delete(struct nvs_fs *fs, uint16_t id)
{
	return nvs_write(fs, id, NULL, 0);
}