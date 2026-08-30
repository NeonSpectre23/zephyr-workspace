uint32_t net_buf_simple_pull_le24(struct net_buf_simple *buf)
{
	struct uint24 {
		uint32_t u24:24;
	} __packed val;

	val = UNALIGNED_GET((struct uint24 *)buf->data);
	net_buf_simple_pull(buf, sizeof(val));

	return sys_le24_to_cpu(val.u24);
}