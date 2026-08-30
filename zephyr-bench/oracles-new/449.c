uint32_t net_buf_simple_remove_le24(struct net_buf_simple *buf)
{
	struct uint24 {
		uint32_t u24 : 24;
	} __packed val;
	void *ptr;

	ptr = net_buf_simple_remove_mem(buf, sizeof(val));
	val = UNALIGNED_GET((struct uint24 *)ptr);

	return sys_le24_to_cpu(val.u24);
}