static inline uint32_t ring_buf_get_claim(struct ring_buf *buf,
					  uint8_t **data,
					  uint32_t size)
{
	uint32_t buf_size = ring_buf_size_get(buf);
	return ring_buf_area_claim(buf, &buf->get, data,
				   MIN(size, buf_size));
}