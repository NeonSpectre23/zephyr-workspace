static inline uint16_t net_buf_remove_be16(struct net_buf *buf)
{
	return net_buf_simple_remove_be16(&buf->b);
}