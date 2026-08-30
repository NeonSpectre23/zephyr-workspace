static inline uint64_t net_buf_pull_le40(struct net_buf *buf)
{
	return net_buf_simple_pull_le40(&buf->b);
}