static inline bool sys_dlist_has_multiple_nodes(const sys_dlist_t *list)
{
	return list->head != list->tail;
}