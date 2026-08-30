static inline struct rbnode *rb_get_min(struct rbtree *tree)
{
	return z_rb_get_minmax(tree, 0U);
}