static inline void sys_dlist_insert_at(sys_dlist_t *list, sys_dnode_t *node,
	int (*cond)(sys_dnode_t *node, void *data), void *data)
{
	if (sys_dlist_is_empty(list)) {
		sys_dlist_append(list, node);
	} else {
		sys_dnode_t *pos = sys_dlist_peek_head(list);

		while ((pos != NULL) && (cond(pos, data) == 0)) {
			pos = sys_dlist_peek_next(list, pos);
		}
		if (pos != NULL) {
			sys_dlist_insert(pos, node);
		} else {
			sys_dlist_append(list, node);
		}
	}
}