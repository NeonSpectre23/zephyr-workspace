uint64_t net_buf_simple_pull_be64(struct net_buf_simple *buf)
{
	uint64_t val;

	val = UNALIGNED_GET((uint64_t *)buf->data);
	net_buf_simple_pull(buf, sizeof(val));

	return sys_be64_to_cpu(val);
}