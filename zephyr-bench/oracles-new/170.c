enum net_verdict ieee802154_handle_ack(struct net_if *iface, struct net_pkt *pkt)
{
	struct ieee802154_context *ctx = net_if_l2_data(iface);

	if (ieee802154_radio_get_hw_capabilities(iface) & IEEE802154_HW_TX_RX_ACK) {
		__ASSERT_NO_MSG(ctx->ack_seq == 0U);
		/* TODO: Release packet in L2 as we're taking ownership. */
		return NET_OK;
	}

	if (pkt->buffer->len == IEEE802154_ACK_PKT_LENGTH) {
		uint8_t len = IEEE802154_ACK_PKT_LENGTH;
		struct ieee802154_fcf_seq *fs;

		fs = ieee802154_validate_fc_seq(net_pkt_data(pkt), NULL, &len);
		if (!fs || fs->fc.frame_type != IEEE802154_FRAME_TYPE_ACK ||
		    fs->sequence != ctx->ack_seq) {
			return NET_CONTINUE;
		}

		k_sem_give(&ctx->ack_lock);

		/* TODO: Release packet in L2 as we're taking ownership. */
		return NET_OK;
	}

	return NET_CONTINUE;
}