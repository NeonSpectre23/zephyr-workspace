static inline void * __must_check net_buf_user_data(const struct net_buf *buf)
{
	return (void *)buf->user_data;
}