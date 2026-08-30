void net_buf_simple_push_le40(struct net_buf_simple *buf, uint64_t val)
{
	NET_BUF_SIMPLE_DBG("buf %p val %" PRIu64, buf, val);

	sys_put_le40(val, net_buf_simple_push(buf, 5));
}