uint32_t net_buf_simple_remove_be32(struct net_buf_simple *buf)
{
	uint32_t val;
	void *ptr;

	ptr = net_buf_simple_remove_mem(buf, sizeof(val));
	val = UNALIGNED_GET((uint32_t *)ptr);

	return sys_be32_to_cpu(val);
}