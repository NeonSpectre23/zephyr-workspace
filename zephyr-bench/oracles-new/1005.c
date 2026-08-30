int uvc_device_shutdown(const struct device *const dev)
{
	struct uvc_data *data = dev->data;

	uvc_deassign_all_descs(dev);

	atomic_clear_bit(&data->state, UVC_STATE_INITIALIZED);

	return 0;
}