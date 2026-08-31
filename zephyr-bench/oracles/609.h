static inline void rb_walk(struct rbtree *tree, rb_visit_t visit_fn,
			   void *cookie)
{
	z_rb_walk(tree->root, visit_fn, cookie);
}