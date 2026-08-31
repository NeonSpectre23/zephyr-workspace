static inline void *net_buf_pull_mem(struct net_buf *buf, size_t len)
{
	return net_buf_simple_pull_mem(&buf->b, len);
}