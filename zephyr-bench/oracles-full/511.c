size_t usb_bos_get_length(void)
{
	return __usb_bos_desc_end - __usb_bos_desc_start;
}