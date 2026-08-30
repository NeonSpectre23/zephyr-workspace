int lorawan_clock_sync_run(void)
{
	ctx.periodicity = CONFIG_LORAWAN_APP_CLOCK_SYNC_PERIODICITY;

	lorawan_register_downlink_callback(&downlink_cb);

	k_work_init_delayable(&ctx.resync_work, clock_sync_resync_handler);
	lorawan_services_reschedule_work(&ctx.resync_work, K_NO_WAIT);

	return 0;
}
