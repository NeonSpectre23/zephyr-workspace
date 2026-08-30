static inline uint8_t net_buf_pull_u8(struct net_buf *buf)
{
	return net_buf_simple_pull_u8(&buf->b);
}