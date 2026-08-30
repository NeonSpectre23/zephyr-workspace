int boot_request_upgrade_multi(int image_index, int permanent)
{
	int rc;

	rc = boot_set_pending_multi(image_index, permanent);
	if (rc) {
		return -EFAULT;
	}
	return 0;
}