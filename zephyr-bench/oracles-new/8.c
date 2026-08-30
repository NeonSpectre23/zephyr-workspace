void *acpi_table_get(char *signature, int inst)
{
	ACPI_STATUS status;
	ACPI_TABLE_HEADER *table;

	if (!acpi.early_init) {
		status = acpi_early_init();
		if (status) {
			LOG_ERR("ACPI early init failed");
			return NULL;
		}
	}

	status = AcpiGetTable(signature, inst, &table);
	if (ACPI_FAILURE(status)) {
		LOG_ERR("ACPI get table failed: %d", status);
		return NULL;
	}

	return (void *)table;
}