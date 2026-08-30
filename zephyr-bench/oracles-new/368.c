void modem_pipelink_release(struct modem_pipelink *link)
{
	K_SPINLOCK(&link->spinlock) {
		link->callback = NULL;
		link->user_data = NULL;
	}
}