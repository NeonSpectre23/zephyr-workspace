static int write_data(const struct shell_transport *transport, const void *data, size_t length,
		      size_t *cnt)
{
	ARG_UNUSED(transport);
	struct shell_mqtt *sh = sh_mqtt;
	int rc = 0;
	struct k_work_sync ws;
	size_t copy_len;

	*cnt = 0;

	/* Not initialized yet */
	if (sh == NULL) {
		return -ENODEV;
	}

	/* Not connected to broker */
	if (sh->transport_state != SHELL_MQTT_TRANSPORT_CONNECTED) {
		goto out;
	}

	(void)k_work_cancel_delayable_sync(&sh->publish_dwork, &ws);

	do {
		if ((sh->tx_buf.len + length - *cnt) > TX_BUF_SIZE) {
			copy_len = TX_BUF_SIZE - sh->tx_buf.len;
		} else {
			copy_len = length - *cnt;
		}

		memcpy(sh->tx_buf.buf + sh->tx_buf.len, (uint8_t *)data + *cnt, copy_len);
		sh->tx_buf.len += copy_len;

		/* Send the data immediately if the buffer is full */
		if (sh->tx_buf.len == TX_BUF_SIZE) {
			rc = sh_mqtt_publish_tx_buf(sh, false);
			if (rc != 0) {
				sh_mqtt_close_and_cleanup(sh);
				(void)sh_mqtt_work_reschedule(&sh->connect_dwork, PROCESS_INTERVAL);
				*cnt = length;
				return rc;
			}
		}

		*cnt += copy_len;
	} while (*cnt < length);

	if (sh->tx_buf.len > 0) {
		(void)sh_mqtt_work_reschedule(&sh->publish_dwork, MQTT_SEND_DELAY_MS);
	}

	/* Inform shell that it is ready for next TX */
	sh->shell_handler(SHELL_TRANSPORT_EVT_TX_RDY, sh->shell_context);

out:
	/* We will always assume that we sent everything */
	*cnt = length;
	return rc;
}