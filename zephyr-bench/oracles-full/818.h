static inline sys_dnode_t *sys_dlist_peek_tail(const sys_dlist_t *list)
{
	return sys_dlist_is_empty(list) ? NULL : list->tail;
}