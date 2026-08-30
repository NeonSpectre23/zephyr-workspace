static inline bool sys_dlist_is_empty(const sys_dlist_t *list)
{
	return list->head == list;
}