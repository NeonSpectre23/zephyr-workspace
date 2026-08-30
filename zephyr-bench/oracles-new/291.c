void lorawan_emul_send_downlink(uint8_t port, bool data_pending, int16_t rssi, int8_t snr,
				uint8_t len, const uint8_t *data)
{
	struct lorawan_downlink_cb *cb;

	/* Iterate over all registered downlink callbacks */
	SYS_SLIST_FOR_EACH_CONTAINER(&dl_callbacks, cb, node) {
		if ((cb->port == LW_RECV_PORT_ANY) || (cb->port == port)) {
			cb->cb(port, data_pending, rssi, snr, len, data);
		}
	}
}