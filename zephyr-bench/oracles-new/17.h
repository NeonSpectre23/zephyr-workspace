static inline const device_handle_t *
device_injected_handles_get(const struct device *dev, size_t *count)
{
	const device_handle_t *rv = dev->deps;
	size_t region = 0;
	size_t i = 0;

	if (rv != NULL) {
		/* Fast forward to injected devices */
		while (region != 1) {
			if (*rv == Z_DEVICE_DEPS_SEP) {
				region++;
			}
			rv++;
		}
		while ((rv[i] != Z_DEVICE_DEPS_ENDS) &&
		       (rv[i] != Z_DEVICE_DEPS_SEP)) {
			++i;
		}
		*count = i;
	}

	return rv;
}