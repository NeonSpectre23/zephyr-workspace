int sensing_get_sensors(int *sensor_nums, const struct sensing_sensor_info **info)
{
	if (info == NULL) {
		LOG_ERR("sensing_sensor_info should not be NULL");
		return -ENODEV;
	}

	STRUCT_SECTION_COUNT(sensing_sensor_info, sensor_nums);

	STRUCT_SECTION_GET(sensing_sensor_info, 0, info);

	return 0;
}