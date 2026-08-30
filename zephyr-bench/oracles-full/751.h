static inline uint32_t net_buf_remove_le24(struct net_buf *buf)
{
	return net_buf_simple_remove_le24(&buf->b);
}