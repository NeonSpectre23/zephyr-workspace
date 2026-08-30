uint64_t net_buf_simple_remove_le48(struct net_buf_simple *buf)
{
	struct uint48 {
		uint64_t u48 : 48;
	} __packed val;
	void *ptr;

	ptr = net_buf_simple_remove_mem(buf, sizeof(val));
	val = UNALIGNED_GET((struct uint48 *)ptr);

	return sys_le48_to_cpu(val.u48);
}