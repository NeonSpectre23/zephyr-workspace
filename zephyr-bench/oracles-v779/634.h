static inline void net_buf_push_le16(struct net_buf *buf, uint16_t val)
{
	net_buf_simple_push_le16(&buf->b, val);
}