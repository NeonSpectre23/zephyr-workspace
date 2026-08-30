static inline void net_buf_push_be32(struct net_buf *buf, uint32_t val)
{
	net_buf_simple_push_be32(&buf->b, val);
}