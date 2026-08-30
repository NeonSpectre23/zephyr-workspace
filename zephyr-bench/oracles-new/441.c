void *net_buf_simple_push_mem(struct net_buf_simple *buf, const void *mem,
			      size_t len)
{
	NET_BUF_SIMPLE_DBG("buf %p len %zu", buf, len);

	return memcpy(net_buf_simple_push(buf, len), mem, len);
}