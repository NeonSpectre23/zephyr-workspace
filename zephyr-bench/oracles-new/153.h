static inline uint16_t net_buf_remove_le16(struct net_buf *buf)
{
	return net_buf_simple_remove_le16(&buf->b);
}