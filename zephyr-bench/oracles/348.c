const struct device *shell_device_filter(size_t idx,
					 shell_device_filter_t filter)
{
	return shell_device_internal(idx, NULL, filter,
				     SHELL_DEVICE_STATUS_READY);
}