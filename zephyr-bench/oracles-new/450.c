uint32_t net_buf_simple_remove_le32(struct net_buf_simple *buf)
{
	uint32_t val;
	void *ptr;

	ptr = net_buf_simple_remove_mem(buf, sizeof(val));
	val = UNALIGNED_GET((uint32_t *)ptr);

	return sys_le32_to_cpu(val);
}