int zms_mount(struct zms_fs *fs)
{
	return zms_mount_internal(fs, false);
}