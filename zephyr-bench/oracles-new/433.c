void net_buf_simple_push_be48(struct net_buf_simple *buf, uint64_t val)
{
	NET_BUF_SIMPLE_DBG("buf %p val %" PRIu64, buf, val);

	sys_put_be48(val, net_buf_simple_push(buf, 6));
}