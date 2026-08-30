void net_buf_simple_add_le64(struct net_buf_simple *buf, uint64_t val)
{
	NET_BUF_SIMPLE_DBG("buf %p val %" PRIu64, buf, val);

	sys_put_le64(val, net_buf_simple_add(buf, sizeof(val)));
}