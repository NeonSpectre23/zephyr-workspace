static inline void *net_buf_push_mem(struct net_buf *buf, const void *mem,
				     size_t len)
{
	return net_buf_simple_push_mem(&buf->b, mem, len);
}