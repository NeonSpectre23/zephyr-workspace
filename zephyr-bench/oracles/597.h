static inline void *net_buf_remove_mem(struct net_buf *buf, size_t len)
{
	return net_buf_simple_remove_mem(&buf->b, len);
}