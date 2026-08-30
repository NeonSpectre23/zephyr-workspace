static inline void net_buf_add_be32(struct net_buf *buf, uint32_t val)
{
	net_buf_simple_add_be32(&buf->b, val);
}