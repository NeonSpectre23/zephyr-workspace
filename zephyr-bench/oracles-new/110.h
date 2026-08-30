static inline void net_buf_add_le64(struct net_buf *buf, uint64_t val)
{
	net_buf_simple_add_le64(&buf->b, val);
}