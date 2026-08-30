static inline void sys_dlist_remove(sys_dnode_t *node)
{
	sys_dnode_t *const prev = node->prev;
	sys_dnode_t *const next = node->next;

	prev->next = next;
	next->prev = prev;
	sys_dnode_init(node);
}