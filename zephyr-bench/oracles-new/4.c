int acpi_device_mmio_get(struct acpi_dev *child_dev, struct acpi_mmio_resource *mmio_res)
{
	ACPI_RESOURCE *res = child_dev->res_lst;
	struct acpi_reg_base *reg_base = mmio_res->reg_base;
	int mmio_cnt = 0;

	do {
		if (!res->Length) {
			LOG_DBG("Found Acpi resource with zero length!");
			break;
		}

		switch (res->Type) {
		case ACPI_RESOURCE_TYPE_IO:
			reg_base[mmio_cnt].type = ACPI_RES_TYPE_IO;
			reg_base[mmio_cnt].port = (uint32_t)res->Data.Io.Minimum;
			reg_base[mmio_cnt++].length = res->Data.Io.AddressLength;
			break;

		case ACPI_RESOURCE_TYPE_FIXED_IO:
			reg_base[mmio_cnt].type = ACPI_RES_TYPE_IO;
			reg_base[mmio_cnt].port = (uint32_t)res->Data.FixedIo.Address;
			reg_base[mmio_cnt++].length = res->Data.FixedIo.AddressLength;
			break;

		case ACPI_RESOURCE_TYPE_MEMORY24:
			reg_base[mmio_cnt].type = ACPI_RES_TYPE_MEM;
			reg_base[mmio_cnt].mmio = (uintptr_t)res->Data.Memory24.Minimum;
			reg_base[mmio_cnt++].length = res->Data.Memory24.AddressLength;
			break;

		case ACPI_RESOURCE_TYPE_MEMORY32:
			reg_base[mmio_cnt].type = ACPI_RES_TYPE_MEM;
			reg_base[mmio_cnt].mmio = (uintptr_t)res->Data.Memory32.Minimum;
			reg_base[mmio_cnt++].length = res->Data.Memory32.AddressLength;
			break;

		case ACPI_RESOURCE_TYPE_FIXED_MEMORY32:
			reg_base[mmio_cnt].type = ACPI_RES_TYPE_MEM;
			reg_base[mmio_cnt].mmio = (uintptr_t)res->Data.FixedMemory32.Address;
			reg_base[mmio_cnt++].length = res->Data.FixedMemory32.AddressLength;
			break;
		case ACPI_RESOURCE_TYPE_ADDRESS64:
			reg_base[mmio_cnt].type = ACPI_RES_TYPE_MEM;
			reg_base[mmio_cnt].mmio = (uintptr_t)res->Data.Address64.Address.Minimum;
			reg_base[mmio_cnt++].length = res->Data.Address64.Address.AddressLength;
			break;
		}

		res = ACPI_NEXT_RESOURCE(res);
		if (mmio_cnt >= mmio_res->mmio_max &&
			 res->Type != ACPI_RESOURCE_TYPE_END_TAG) {
			return -ENOMEM;
		}
	} while (res->Type != ACPI_RESOURCE_TYPE_END_TAG);

	if (!mmio_cnt) {
		return -ENODEV;
	}

	mmio_res->mmio_max = mmio_cnt;

	return 0;
}