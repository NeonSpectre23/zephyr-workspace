osStatus osMailFree(osMailQId queue_id, void *mail)
{
	osMailQDef_t *queue_def = (osMailQDef_t *)queue_id;

	k_mem_slab_free((struct k_mem_slab *)(queue_def->pool), (void *)mail);

	return osOK;
}