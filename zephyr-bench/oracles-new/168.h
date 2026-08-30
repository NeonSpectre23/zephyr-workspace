static inline void net_pkt_set_ieee802154_rssi_dbm(struct net_pkt *pkt, int16_t rssi)
{
	if (likely(rssi >= IEEE802154_MAC_RSSI_DBM_MIN && rssi <= IEEE802154_MAC_RSSI_DBM_MAX)) {
		int16_t unsigned_rssi = rssi - IEEE802154_MAC_RSSI_DBM_MIN;

		net_pkt_cb_ieee802154(pkt)->rssi = unsigned_rssi;
		return;
	} else if (rssi == IEEE802154_MAC_RSSI_DBM_UNDEFINED) {
		net_pkt_cb_ieee802154(pkt)->rssi = IEEE802154_MAC_RSSI_UNDEFINED;
		return;
	} else if (rssi < IEEE802154_MAC_RSSI_DBM_MIN) {
		net_pkt_cb_ieee802154(pkt)->rssi = IEEE802154_MAC_RSSI_MIN;
		return;
	} else if (rssi > IEEE802154_MAC_RSSI_DBM_MAX) {
		net_pkt_cb_ieee802154(pkt)->rssi = IEEE802154_MAC_RSSI_MAX;
		return;
	}

	CODE_UNREACHABLE;
}

#if 