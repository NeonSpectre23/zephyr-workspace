int acpi_dmar_ioapic_get(uint16_t *ioapic_id)
{
	ACPI_TABLE_DMAR *dmar = acpi_table_get("DMAR", 0);
	uint16_t found_ioapic = USHRT_MAX;

	if (dmar == NULL) {
		return -ENODEV;
	}

	acpi_dmar_foreach_subtable(dmar, subtable_handler, &found_ioapic);
	if (found_ioapic != USHRT_MAX) {
		*ioapic_id = found_ioapic;
		return 0;
	}

	return -ENOENT;
}