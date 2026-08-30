static inline void *net_buf_add(struct net_buf *buf, size_t len)
{
	return net_buf_simple_add(&buf->b, len);
}