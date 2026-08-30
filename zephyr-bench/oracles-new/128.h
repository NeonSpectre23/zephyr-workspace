static inline uint32_t net_buf_pull_le32(struct net_buf *buf)
{
	return net_buf_simple_pull_le32(&buf->b);
}