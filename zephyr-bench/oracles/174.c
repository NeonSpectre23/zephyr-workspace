void modem_pipelink_attach(struct modem_pipelink *link,
			   modem_pipelink_callback callback,
			   void *user_data)
{
	K_SPINLOCK(&link->spinlock) {
		link->callback = callback;
		link->user_data = user_data;
	}
}