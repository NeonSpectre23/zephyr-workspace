void *net_buf_simple_remove_mem(struct net_buf_simple *buf, size_t len)
{
	NET_BUF_SIMPLE_DBG("buf %p len %zu", buf, len);

	__ASSERT_NO_MSG(buf->len >= len);

	buf->len -= len;
	return buf->data + buf->len;
}