const struct device *shell_device_lookup(size_t idx,
					 const char *prefix)
{
	return shell_device_internal(idx, prefix, NULL,
				     SHELL_DEVICE_STATUS_READY);
}