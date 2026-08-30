void modem_pipelink_notify_connected(struct modem_pipelink *link)
{
	K_SPINLOCK(&link->spinlock) {
		if (link->connected) {
			K_SPINLOCK_BREAK;
		}

		link->connected = true;
		try_callback(link, MODEM_PIPELINK_EVENT_CONNECTED);
	}
}