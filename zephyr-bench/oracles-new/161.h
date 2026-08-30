static inline void net_buf_simple_reset(struct net_buf_simple *buf)
{
	buf->len  = 0U;
	buf->data = buf->__buf;
}