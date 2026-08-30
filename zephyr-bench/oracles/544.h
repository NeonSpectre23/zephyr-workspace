static inline void net_buf_add_be48(struct net_buf *buf, uint64_t val)
{
	net_buf_simple_add_be48(&buf->b, val);
}