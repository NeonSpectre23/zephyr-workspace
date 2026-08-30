void rb_insert(struct rbtree *tree, struct rbnode *node)
{
	set_child(node, 0U, NULL);
	set_child(node, 1U, NULL);

	if (tree->root == NULL) {
		tree->root = node;
		tree->max_depth = 1;
		set_color(node, BLACK);
		return;
	}

#ifdef CONFIG_MISRA_SANE
	struct rbnode **stack = &tree->iter_stack[0];
#else
	struct rbnode *stack[tree->max_depth + 1];
#endif

	int stacksz = find_and_stack(tree, node, stack);

	struct rbnode *parent = stack[stacksz - 1];

	uint8_t side = tree->lessthan_fn(node, parent) ? 0U : 1U;

	set_child(parent, side, node);
	set_color(node, RED);

	stack[stacksz] = node;
	++stacksz;
	fix_extra_red(stack, stacksz);

	if (stacksz > tree->max_depth) {
		tree->max_depth = stacksz;
	}

	/* We may have rotated up into the root! */
	tree->root = stack[0];
	CHECK(is_black(tree->root));
}