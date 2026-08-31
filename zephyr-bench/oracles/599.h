static inline size_t net_buf_tailroom(const struct net_buf *buf)
{
	return net_buf_simple_tailroom(&buf->b);
}