static inline void net_buf_push_le32(struct net_buf *buf, uint32_t val)
{
	net_buf_simple_push_le32(&buf->b, val);
}