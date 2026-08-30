static inline void ring_buf_item_init(struct ring_buf *buf,
				      uint32_t size,
				      uint32_t *data)
{
	__ASSERT(size <= RING_BUFFER_MAX_SIZE / 4, RING_BUFFER_SIZE_ASSERT_MSG);
	ring_buf_init(buf, 4 * size, (uint8_t *)data);
}