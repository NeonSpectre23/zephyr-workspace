static inline void net_buf_reserve(struct net_buf *buf, size_t reserve)
{
	net_buf_simple_reserve(&buf->b, reserve);
}