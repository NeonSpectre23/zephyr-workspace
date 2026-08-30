static inline size_t net_buf_headroom(const struct net_buf *buf)
{
	return net_buf_simple_headroom(&buf->b);
}