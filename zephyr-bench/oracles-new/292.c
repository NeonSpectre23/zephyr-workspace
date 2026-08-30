int lorawan_frag_transport_run(void (*transport_finished_cb)(void))
{
	finished_cb = transport_finished_cb;

	lorawan_register_downlink_callback(&downlink_cb);

	return 0;
}