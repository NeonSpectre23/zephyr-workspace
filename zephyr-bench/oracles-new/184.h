static inline struct rbnode *rb_get_max(struct rbtree *tree)
{
	return z_rb_get_minmax(tree, 1U);
}