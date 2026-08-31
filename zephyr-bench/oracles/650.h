static inline size_t sys_dlist_len(const sys_dlist_t *list)
{
	size_t len = 0;
	sys_dnode_t *node = NULL;

	SYS_DLIST_FOR_EACH_NODE(list, node) {
		len++;
	}
	return len;
}