uint64_t net_buf_simple_remove_be40(struct net_buf_simple *buf)
{
	struct uint40 {
		uint64_t u40: 40;
	} __packed val;
	void *ptr;

	ptr = net_buf_simple_remove_mem(buf, sizeof(val));
	val = UNALIGNED_GET((struct uint40 *)ptr);

	return sys_be40_to_cpu(val.u40);
}