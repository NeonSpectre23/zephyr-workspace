int boot_write_img_confirmed_multi(int image_index)
{
	int rc;

	rc = boot_set_confirmed_multi(image_index);
	if (rc) {
		return -EIO;
	}

	return 0;
}