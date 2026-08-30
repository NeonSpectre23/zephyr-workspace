void net_buf_simple_push_be32(struct net_buf_simple *buf, uint32_t val)
{
	NET_BUF_SIMPLE_DBG("buf %p val %u", buf, val);

	sys_put_be32(val, net_buf_simple_push(buf, sizeof(val)));
}