static inline void net_buf_push_be24(struct net_buf *buf, uint32_t val)
{
	net_buf_simple_push_be24(&buf->b, val);
}