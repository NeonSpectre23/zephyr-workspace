bool rb_contains(struct rbtree *tree, struct rbnode *node)
{
	struct rbnode *n = tree->root;

	while ((n != NULL) && (n != node)) {
		n = get_child(n, tree->lessthan_fn(n, node));
	}

	return n == node;
}