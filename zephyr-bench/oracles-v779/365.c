static inline int send_sf(struct isotp_send_ctx *sctx)
{
	struct can_frame frame;
	size_t len = get_send_ctx_data_len(sctx);
	int index = 0;
	int ret;
	const uint8_t *data;

	prepare_frame(&frame, &sctx->tx_addr);

	data = get_send_ctx_data(sctx);
	pull_send_ctx_data(sctx, len);

	if ((sctx->tx_addr.flags & ISOTP_MSG_EXT_ADDR) != 0) {
		frame.data[index++] = sctx->tx_addr.ext_addr;
	}

	if (IS_ENABLED(CONFIG_CAN_FD_MODE) && (sctx->tx_addr.flags & ISOTP_MSG_FDF) != 0 &&
	    len > ISOTP_4BIT_SF_MAX_CAN_DL - 1 - index) {
		frame.data[index++] = ISOTP_PCI_TYPE_SF;
		frame.data[index++] = len;
	} else {
		frame.data[index++] = ISOTP_PCI_TYPE_SF | len;
	}

	if (len > sctx->tx_addr.dl - index) {
		LOG_ERR("SF len does not fit DL");
		return -ENOSPC;
	}

	memcpy(&frame.data[index], data, len);

	if (IS_ENABLED(CONFIG_ISOTP_ENABLE_TX_PADDING) ||
	    (IS_ENABLED(CONFIG_CAN_FD_MODE) && (sctx->tx_addr.flags & ISOTP_MSG_FDF) != 0 &&
	     len + index > ISOTP_PADDED_FRAME_DL_MIN)) {
		/* AUTOSAR requirements SWS_CanTp_00348 / SWS_CanTp_00351.
		 * Mandatory for ISO-TP CAN FD frames > 8 bytes.
		 */
		frame.dlc = can_bytes_to_dlc(
			MAX(ISOTP_PADDED_FRAME_DL_MIN, len + index));
		memset(&frame.data[index + len], ISOTP_PAD_BYTE,
		       can_dlc_to_bytes(frame.dlc) - len - index);
	} else {
		frame.dlc = can_bytes_to_dlc(len + index);
	}

	sctx->state = ISOTP_TX_SEND_SF;
	ret = can_send(sctx->can_dev, &frame, K_MSEC(ISOTP_A_TIMEOUT_MS), send_can_tx_cb, sctx);
	return ret;
}