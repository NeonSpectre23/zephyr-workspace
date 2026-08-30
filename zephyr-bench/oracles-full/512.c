void usb_bos_register_cap(void *desc)
{
	ARG_UNUSED(desc);

	/* Has effect only on first register */
	bos_hdr.wTotalLength = usb_bos_get_length();

	bos_hdr.bNumDeviceCaps += 1U;
}