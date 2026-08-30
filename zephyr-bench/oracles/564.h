static inline uint16_t net_buf_pull_le16(struct net_buf *buf)
{
	return net_buf_simple_pull_le16(&buf->b);
}