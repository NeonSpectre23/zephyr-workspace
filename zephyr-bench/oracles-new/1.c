int acpi_current_resource_get(char *dev_name, ACPI_RESOURCE **res)
{
	ACPI_BUFFER rt_buffer;
	ACPI_NAMESPACE_NODE *node;
	ACPI_STATUS status;

	LOG_DBG("%s", dev_name);

	status = check_init_status();
	if (status) {
		return -EAGAIN;
	}

	node = acpi_evaluate_method(dev_name, METHOD_NAME__CRS);
	if (!node) {
		LOG_ERR("Evaluation failed for given device: %s", dev_name);
		return -ENOTSUP;
	}

	rt_buffer.Pointer = NULL;
	rt_buffer.Length = ACPI_ALLOCATE_LOCAL_BUFFER;

	status = AcpiGetCurrentResources(node, &rt_buffer);
	if (ACPI_FAILURE(status)) {
		LOG_ERR("AcpiGetCurrentResources failed: %s", AcpiFormatException(status));
		return -ENOTSUP;
	}

	*res = rt_buffer.Pointer;

	return 0;
}