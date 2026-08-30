uint64_t net_buf_simple_pull_be40(struct net_buf_simple *buf)
{
	struct uint40 {
		uint64_t u40: 40;
	} __packed val;

	val = UNALIGNED_GET((struct uint40 *)buf->data);
	net_buf_simple_pull(buf, sizeof(val));

	return sys_be40_to_cpu(val.u40);
}