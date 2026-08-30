static inline void sys_dlist_prepend(sys_dlist_t *list, sys_dnode_t *node)
{
	sys_dnode_t *const head = list->head;

	node->next = head;
	node->prev = list;

	head->prev = node;
	list->head = node;
}