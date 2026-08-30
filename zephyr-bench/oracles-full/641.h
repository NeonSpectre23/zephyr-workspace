static inline const struct device *
device_from_handle(device_handle_t dev_handle)
{
	STRUCT_SECTION_START_EXTERN(device);
	const struct device *dev = NULL;
	size_t numdev;

	STRUCT_SECTION_COUNT(device, &numdev);

	if ((dev_handle > 0) && ((size_t)dev_handle <= numdev)) {
		dev = &STRUCT_SECTION_START(device)[dev_handle - 1];
	}

	return dev;
}