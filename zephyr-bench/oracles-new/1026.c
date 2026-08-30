struct rbnode *z_rb_child(struct rbnode *node, uint8_t side)
{
	return get_child(node, side);
}