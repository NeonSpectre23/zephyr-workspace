static inline uint8_t *net_buf_add_u8(struct net_buf *buf, uint8_t val)
{
	return net_buf_simple_add_u8(&buf->b, val);
}