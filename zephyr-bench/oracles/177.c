void modem_pipelink_notify_disconnected(struct modem_pipelink *link)
{
	K_SPINLOCK(&link->spinlock) {
		if (!link->connected) {
			K_SPINLOCK_BREAK;
		}

		link->connected = false;
		try_callback(link, MODEM_PIPELINK_EVENT_DISCONNECTED);
	}
}