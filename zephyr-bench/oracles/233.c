osStatus_t osKernelGetInfo(osVersion_t *version, char *id_buf, uint32_t id_size)
{
	uint32_t ver = sys_kernel_version_get();

	if (version != NULL) {
		version->api = ver;
		version->kernel = ver;
	}

	if ((id_buf != NULL) && (version != NULL)) {
		snprintf(id_buf, id_size, "Zephyr V%2" PRIu32 ".%2" PRIu32 ".%2" PRIu32,
			 SYS_KERNEL_VER_MAJOR(version->kernel),
			 SYS_KERNEL_VER_MINOR(version->kernel),
			 SYS_KERNEL_VER_PATCHLEVEL(version->kernel));
	}

	return osOK;
}