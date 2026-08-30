bool modem_pipelink_is_connected(struct modem_pipelink *link)
{
	bool connected = false;

	K_SPINLOCK(&link->spinlock) {
		connected = link->connected;
	}

	return connected;
}