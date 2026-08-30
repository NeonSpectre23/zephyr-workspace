void net_buf_simple_reserve(struct net_buf_simple *buf, size_t reserve)
{
	__ASSERT_NO_MSG(buf);
	__ASSERT_NO_MSG(buf->len == 0U);
	NET_BUF_SIMPLE_DBG("buf %p reserve %zu", buf, reserve);

	buf->data = buf->__buf + reserve;
}