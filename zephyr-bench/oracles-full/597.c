const struct zbus_channel *zbus_chan_from_name(const char *name)
{
	CHECKIF(name == NULL) {
		return NULL;
	}

	STRUCT_SECTION_FOREACH(zbus_channel, chan) {
		if (strcmp(chan->name, name) == 0) {
			/* Found matching channel */
			return chan;
		}
	}
	/* No matching channel exists */
	return NULL;
}