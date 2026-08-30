struct sys_set_node *sys_set_find(struct sys_set_node *node)
{
	struct sys_set_node *parent;

	while (node != node->parent) {
		parent = node->parent;
		node->parent = parent->parent;
		node = parent;
	}

	return node;
}