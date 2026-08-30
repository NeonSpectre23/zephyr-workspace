const struct zbus_channel *zbus_chan_from_id(uint32_t channel_id)
{
	if (channel_id == ZBUS_CHAN_ID_INVALID) {
		return NULL;
	}
	STRUCT_SECTION_FOREACH(zbus_channel, chan) {
		if (chan->id == channel_id) {
			/* Found matching channel */
			return chan;
		}
	}
	/* No matching channel exists */
	return NULL;
}