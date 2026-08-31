static inline void sys_dlist_insert(sys_dnode_t *successor, sys_dnode_t *node)
{
	sys_dnode_t *const prev = successor->prev;

	node->prev = prev;
	node->next = successor;
	prev->next = node;
	successor->prev = node;
}