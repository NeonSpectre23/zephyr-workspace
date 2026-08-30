static inline const device_handle_t *
device_supported_handles_get(const struct device *dev, size_t *count)
{
	const device_handle_t *rv = dev->deps;
	size_t region = 0;
	size_t i = 0;

	if (rv != NULL) {
		/* Fast forward to supporting devices */
		while (region != 2) {
			if (*rv == Z_DEVICE_DEPS_SEP) {
				region++;
			}
			rv++;
		}
		/* Count supporting devices.
		 * Trailing NULL's can be injected by gen_device_deps.py due to
		 * CONFIG_PM_DEVICE_POWER_DOMAIN_DYNAMIC_NUM
		 */
		while ((rv[i] != Z_DEVICE_DEPS_ENDS) &&
		       (rv[i] != DEVICE_HANDLE_NULL)) {
			++i;
		}
		*count = i;
	}

	return rv;
}