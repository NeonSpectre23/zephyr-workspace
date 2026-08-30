void net_buf_simple_add_le40(struct net_buf_simple *buf, uint64_t val)
{
	NET_BUF_SIMPLE_DBG("buf %p val %" PRIu64, buf, val);

	sys_put_le40(val, net_buf_simple_add(buf, 5));
}