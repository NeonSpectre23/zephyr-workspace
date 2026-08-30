static inline uint16_t net_buf_pull_be16(struct net_buf *buf)
{
	return net_buf_simple_pull_be16(&buf->b);
}