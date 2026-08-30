void acpi_dmar_foreach_subtable(ACPI_TABLE_DMAR *dmar,
				dmar_foreach_subtable_func_t func, void *arg)
{
	uint16_t length = dmar->Header.Length;
	uintptr_t offset = sizeof(ACPI_TABLE_DMAR);

	__ASSERT_NO_MSG(length >= offset);

	while (offset < length) {
		ACPI_DMAR_HEADER *subtable = ACPI_ADD_PTR(ACPI_DMAR_HEADER, dmar, offset);

		__ASSERT_NO_MSG(subtable->Length >= sizeof(*subtable));
		__ASSERT_NO_MSG(subtable->Length <= length - offset);

		func(subtable, arg);

		offset += subtable->Length;
	}
}