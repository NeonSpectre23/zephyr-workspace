int zms_mount_force(struct zms_fs *fs)
{
	return zms_mount_internal(fs, true);
}