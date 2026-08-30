void acpi_dmar_foreach_devscope(ACPI_DMAR_HARDWARE_UNIT *hu,
				dmar_foreach_devscope_func_t func, void *arg)
{
	uint16_t length = hu->Header.Length;
	uintptr_t offset = sizeof(ACPI_DMAR_HARDWARE_UNIT);

	__ASSERT_NO_MSG(length >= offset);

	while (offset < length) {
		ACPI_DMAR_DEVICE_SCOPE *devscope = ACPI_ADD_PTR(ACPI_DMAR_DEVICE_SCOPE,
								hu, offset);

		__ASSERT_NO_MSG(devscope->Length >= sizeof(*devscope));
		__ASSERT_NO_MSG(devscope->Length <= length - offset);

		func(devscope, arg);

		offset += devscope->Length;
	}
}