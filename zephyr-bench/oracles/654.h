static inline sys_dnode_t *sys_dlist_peek_prev(const sys_dlist_t *list,
					       const sys_dnode_t *node)
{
	return (node != NULL) ? sys_dlist_peek_prev_no_check(list, node) : NULL;
}