uint16_t net_buf_simple_remove_be16(struct net_buf_simple *buf)
{
	uint16_t val;
	void *ptr;

	ptr = net_buf_simple_remove_mem(buf, sizeof(val));
	val = UNALIGNED_GET((uint16_t *)ptr);

	return sys_be16_to_cpu(val);
}