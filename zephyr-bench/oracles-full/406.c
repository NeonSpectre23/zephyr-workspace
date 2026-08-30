void rb_remove(struct rbtree *tree, struct rbnode *node)
{
	struct rbnode *tmp;
#ifdef CONFIG_MISRA_SANE
	struct rbnode **stack = &tree->iter_stack[0];
#else
	struct rbnode *stack[tree->max_depth + 1];
#endif

	int stacksz = find_and_stack(tree, node, stack);

	if (node != stack[stacksz - 1]) {
		return;
	}

	/* We can only remove a node with zero or one child, if we
	 * have two then pick the "biggest" child of side 0 (smallest
	 * of 1 would work too) and swap our spot in the tree with
	 * that one
	 */
	if ((get_child(node, 0U) != NULL) && (get_child(node, 1U) != NULL)) {
		int stacksz0 = stacksz;
		struct rbnode *hiparent, *loparent;
		struct rbnode *node2 = get_child(node, 0U);

		hiparent = (stacksz > 1) ? stack[stacksz - 2] : NULL;
		stack[stacksz] = node2;
		++stacksz;
		while (get_child(node2, 1U) != NULL) {
			node2 = get_child(node2, 1U);
			stack[stacksz] = node2;
			++stacksz;
		}

		loparent = stack[stacksz - 2];

		/* Now swap the position of node/node2 in the tree.
		 * Design note: this is a spot where being an
		 * intrusive data structure hurts us fairly badly.
		 * The trees you see in textbooks do this by swapping
		 * the "data" pointers between the two nodes, but we
		 * have a few special cases to check.  In principle
		 * this works by swapping the child pointers between
		 * the nodes and retargeting the nodes pointing to
		 * them from their parents, but: (1) the upper node
		 * may be the root of the tree and not have a parent,
		 * and (2) the lower node may be a direct child of the
		 * upper node.  Remember to swap the color bits of the
		 * two nodes also.  And of course we don't have parent
		 * pointers, so the stack tracking this structure
		 * needs to be swapped too!
		 */
		if (hiparent != NULL) {
			set_child(hiparent, get_side(hiparent, node), node2);
		} else {
			tree->root = node2;
		}

		if (loparent == node) {
			set_child(node, 0U, get_child(node2, 0U));
			set_child(node2, 0U, node);
		} else {
			set_child(loparent, get_side(loparent, node2), node);
			tmp = get_child(node, 0U);
			set_child(node, 0U, get_child(node2, 0U));
			set_child(node2, 0U, tmp);
		}

		set_child(node2, 1U, get_child(node, 1U));
		set_child(node, 1U, NULL);

		tmp = stack[stacksz0 - 1];
		stack[stacksz0 - 1] = stack[stacksz - 1];
		stack[stacksz - 1] = tmp;

		enum rb_color ctmp = get_color(node);

		set_color(node, get_color(node2));
		set_color(node2, ctmp);
	}

	CHECK((get_child(node, 0U) == NULL) ||
	      (get_child(node, 1U) == NULL));

	struct rbnode *child = get_child(node, 0U);

	if (child == NULL) {
		child = get_child(node, 1U);
	}

	/* Removing the root */
	if (stacksz < 2) {
		tree->root = child;
		if (child != NULL) {
			set_color(child, BLACK);
		} else {
			tree->max_depth = 0;
		}
		return;
	}

	struct rbnode *parent = stack[stacksz - 2];

	/* Special case: if the node to be removed is childless, then
	 * we leave it in place while we do the missing black
	 * rotations, which will replace it with a proper NULL when
	 * they isolate it.
	 */
	if (child == NULL) {
		if (is_black(node)) {
			fix_missing_black(stack, stacksz, node);
		} else {
			/* Red childless nodes can just be dropped */
			set_child(parent, get_side(parent, node), NULL);
		}
	} else {
		set_child(parent, get_side(parent, node), child);

		/* Check colors, if one was red (at least one must have been
		 * black in a valid tree), then we're done.
		 */
		__ASSERT(is_black(node) || is_black(child), "both nodes red?!");
		if (is_red(node) || is_red(child)) {
			set_color(child, BLACK);
		}
	}

	/* We may have rotated up into the root! */
	tree->root = stack[0];
}