static inline uint64_t net_buf_remove_le64(struct net_buf *buf)
{
	return net_buf_simple_remove_le64(&buf->b);
}