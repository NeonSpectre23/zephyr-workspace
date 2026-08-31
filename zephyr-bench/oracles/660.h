static inline bool sys_dnode_is_linked(const sys_dnode_t *node)
{
	return node->next != NULL;
}