static inline void ring_buf_init(struct ring_buf *buf,
				 uint32_t size,
				 uint8_t *data)
{
	__ASSERT(size <= RING_BUFFER_MAX_SIZE, RING_BUFFER_SIZE_ASSERT_MSG);

	buf->size = size;
	buf->buffer = data;
	ring_buf_internal_reset(buf, 0);
}