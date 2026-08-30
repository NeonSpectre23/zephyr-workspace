static inline void net_buf_add_le24(struct net_buf *buf, uint32_t val)
{
	net_buf_simple_add_le24(&buf->b, val);
}