static inline sys_dnode_t *sys_dlist_get(sys_dlist_t *list)
{
	sys_dnode_t *node = NULL;

	if (!sys_dlist_is_empty(list)) {
		node = list->head;
		sys_dlist_remove(node);
	}

	return node;
}