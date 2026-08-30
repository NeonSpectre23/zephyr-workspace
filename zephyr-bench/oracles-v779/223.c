void net_buf_reset(struct net_buf *buf)
{
	__ASSERT_NO_MSG(buf->frags == NULL);

	net_buf_simple_reset(&buf->b);
}