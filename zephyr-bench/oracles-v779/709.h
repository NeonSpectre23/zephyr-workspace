static inline bool sys_dlist_is_tail(const sys_dlist_t *list, const sys_dnode_t *node)
{
	return list->tail == node;
}