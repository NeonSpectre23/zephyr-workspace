static inline void sys_dlist_append(sys_dlist_t *list, sys_dnode_t *node)
{
	sys_dnode_t *const tail = list->tail;

	node->next = list;
	node->prev = tail;

	tail->next = node;
	list->tail = node;
}