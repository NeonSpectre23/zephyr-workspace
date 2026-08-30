void *net_buf_simple_pull_mem(struct net_buf_simple *buf, size_t len)
{
	void *data = buf->data;

	NET_BUF_SIMPLE_DBG("buf %p len %zu", buf, len);

	__ASSERT_NO_MSG(buf->len >= len);

	buf->len -= len;
	buf->data += len;

	return data;
}