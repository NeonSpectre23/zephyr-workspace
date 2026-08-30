static inline void net_buf_push_le40(struct net_buf *buf, uint64_t val)
{
	net_buf_simple_push_le40(&buf->b, val);
}