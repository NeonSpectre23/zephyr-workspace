static inline void sys_set_makeset(struct sys_set_node *node, uint16_t rank)
{
	node->parent = node;
	node->rank = rank;
}