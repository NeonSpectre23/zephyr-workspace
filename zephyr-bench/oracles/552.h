static inline void *net_buf_add_mem(struct net_buf *buf, const void *mem,
				    size_t len)
{
	return net_buf_simple_add_mem(&buf->b, mem, len);
}