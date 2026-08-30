static inline uint32_t ring_buf_item_space_get(const struct ring_buf *buf)
{
	return ring_buf_space_get(buf) / 4;
}