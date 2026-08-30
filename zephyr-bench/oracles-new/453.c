uint64_t net_buf_simple_remove_le64(struct net_buf_simple *buf)
{
	uint64_t val;
	void *ptr;

	ptr = net_buf_simple_remove_mem(buf, sizeof(val));
	val = UNALIGNED_GET((uint64_t *)ptr);

	return sys_le64_to_cpu(val);
}