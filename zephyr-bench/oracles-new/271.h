static inline void sys_slist_init(sys_slist_t *list)
{
	list->head = NULL;
	list->tail = NULL;
}