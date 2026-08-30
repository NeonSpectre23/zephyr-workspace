int boot_request_upgrade(int permanent)
{
#ifdef FLASH_AREA_IMAGE_SECONDARY
	int rc;

	rc = boot_set_pending(permanent);
	if (rc) {
		return -EFAULT;
	}
#endif /* FLASH_AREA_IMAGE_SECONDARY */
	return 0;
}