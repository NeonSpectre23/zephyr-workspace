uint64_t net_buf_simple_pull_be48(struct net_buf_simple *buf)
{
	struct uint48 {
		uint64_t u48:48;
	} __packed val;

	val = UNALIGNED_GET((struct uint48 *)buf->data);
	net_buf_simple_pull(buf, sizeof(val));

	return sys_be48_to_cpu(val.u48);
}