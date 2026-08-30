static inline const device_handle_t *
device_required_handles_get(const struct device *dev, size_t *count)
{
	const device_handle_t *rv = dev->deps;

	if (rv != NULL) {
		size_t i = 0;

		while ((rv[i] != Z_DEVICE_DEPS_ENDS) &&
		       (rv[i] != Z_DEVICE_DEPS_SEP)) {
			++i;
		}
		*count = i;
	}

	return rv;
}