struct acpi_dev *acpi_device_get(const char *hid, const char *uid)
{
	struct acpi_dev *child_dev;
	int i = 0;

	LOG_DBG("");

	if (check_init_status()) {
		return NULL;
	}

	do {
		child_dev = &acpi.child_dev[i];
		if (!child_dev->path) {
			LOG_DBG("NULL device path found");
			continue;
		}

		if (!child_dev->res_lst || !child_dev->dev_info ||
		    !child_dev->dev_info->HardwareId.Length) {
			continue;
		}

		if (!strcmp(hid, child_dev->dev_info->HardwareId.String)) {
			if (uid && child_dev->dev_info->UniqueId.Length) {
				if (!strcmp(child_dev->dev_info->UniqueId.String, uid)) {
					return child_dev;
				}
			} else {
				return child_dev;
			}
		}
	} while (i++ < acpi.num_dev);

	return NULL;
}