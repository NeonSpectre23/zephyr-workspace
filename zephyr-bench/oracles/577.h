static inline void net_buf_push_be64(struct net_buf *buf, uint64_t val)
{
	net_buf_simple_push_be64(&buf->b, val);
}