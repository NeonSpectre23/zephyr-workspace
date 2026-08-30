int lvgl_init(void)
{
	const struct device *display_dev[DT_ZEPHYR_DISPLAYS_COUNT];
	struct lvgl_disp_data *p_disp_data;
	int err;

	/* clang-format off */
	FOR_EACH(ENUMERATE_DISPLAY_DEVS, (), LV_DISPLAYS_IDX_LIST);
	/* clang-format on */
	for (int i = 0; i < DT_ZEPHYR_DISPLAYS_COUNT; i++) {
		if (!device_is_ready(display_dev[i])) {
			LOG_ERR("Display device %d is not ready", i);
			return -ENODEV;
		}
	}

	lv_init();
	lv_tick_set_cb(k_uptime_get_32);

#if CONFIG_LV_Z_LOG_LEVEL != 0
	lv_log_register_print_cb(lvgl_log);
#endif

#ifdef CONFIG_LV_Z_USE_FILESYSTEM
	lvgl_fs_init();
#endif

#ifdef CONFIG_LV_Z_BUFFER_ALLOC_STATIC
	/* clang-format off */
	FOR_EACH(LV_BUFFERS_REFERENCES, (), LV_DISPLAYS_IDX_LIST);
	/* clang-format on */
#endif

	for (int i = 0; i < DT_ZEPHYR_DISPLAYS_COUNT; i++) {
		p_disp_data = &disp_data[i];
		p_disp_data->display_dev = display_dev[i];
		display_get_capabilities(display_dev[i], &p_disp_data->cap);

		lv_displays[i] = lv_display_create(p_disp_data->cap.x_resolution,
						   p_disp_data->cap.y_resolution);
		if (!lv_displays[i]) {
			LOG_ERR("Failed to create display %d LV object.", i);
			return -ENOMEM;
		}

		lv_display_set_user_data(lv_displays[i], p_disp_data);
		if (set_lvgl_rendering_cb(lv_displays[i]) != 0) {
			LOG_ERR("Display %d not supported.", i);
			return -ENOTSUP;
		}

#ifdef CONFIG_LV_Z_BUFFER_ALLOC_STATIC
		lvgl_allocate_rendering_buffers_static(lv_displays[i], i);
#else
		err = lvgl_allocate_rendering_buffers(lv_displays[i]);
		if (err < 0) {
			return err;
		}
#endif

#ifdef CONFIG_LV_Z_FULL_REFRESH
		lv_display_set_render_mode(lv_displays[i], LV_DISPLAY_RENDER_MODE_FULL);
#endif
	}

	err = lvgl_init_input_devices();
	if (err < 0) {
		LOG_ERR("Failed to initialize input devices.");
		return err;
	}

#ifdef CONFIG_LV_Z_RUN_LVGL_ON_WORKQUEUE
	const struct k_work_queue_config lvgl_workqueue_cfg = {
		.name = "lvgl",
	};

	k_work_queue_init(&lvgl_workqueue);
	k_work_queue_start(&lvgl_workqueue, lvgl_workqueue_stack,
			   K_THREAD_STACK_SIZEOF(lvgl_workqueue_stack),
			   CONFIG_LV_Z_LVGL_WORKQUEUE_PRIORITY, &lvgl_workqueue_cfg);

	k_work_submit_to_queue(&lvgl_workqueue, &lvgl_work.work);
#endif

	return 0;
}