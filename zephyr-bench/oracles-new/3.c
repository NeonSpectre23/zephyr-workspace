int acpi_device_irq_get(struct acpi_dev *child_dev, struct acpi_irq_resource *irq_res)
{
	ACPI_RESOURCE *res = acpi_resource_parse(child_dev->res_lst, ACPI_RESOURCE_TYPE_IRQ);

	if (!res) {
		res = acpi_resource_parse(child_dev->res_lst, ACPI_RESOURCE_TYPE_EXTENDED_IRQ);
		if (!res) {
			return -ENODEV;
		}

		if (res->Data.ExtendedIrq.InterruptCount > irq_res->irq_vector_max) {
			return -ENOMEM;
		}

		irq_res->irq_vector_max = res->Data.ExtendedIrq.InterruptCount;
		for (int i = 0; i < irq_res->irq_vector_max; i++) {
			irq_res->irqs[i] = (uint16_t)res->Data.ExtendedIrq.Interrupts[i];
		}

		irq_res->flags = arch_acpi_encode_irq_flags(res->Data.ExtendedIrq.Polarity,
							    res->Data.ExtendedIrq.Triggering);
	} else {
		if (res->Data.Irq.InterruptCount > irq_res->irq_vector_max) {
			return -ENOMEM;
		}

		irq_res->irq_vector_max = res->Data.Irq.InterruptCount;
		for (int i = 0; i < irq_res->irq_vector_max; i++) {
			irq_res->irqs[i] = (uint16_t)res->Data.Irq.Interrupts[i];
		}

		irq_res->flags = arch_acpi_encode_irq_flags(res->Data.ExtendedIrq.Polarity,
							    res->Data.ExtendedIrq.Triggering);
	}

	return 0;
}