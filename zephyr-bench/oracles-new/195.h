static inline uint32_t ring_buf_put_claim(struct ring_buf *buf,
					  uint8_t **data,
					  uint32_t size)
{
	uint32_t space = ring_buf_space_get(buf);
	return ring_buf_area_claim(buf, &buf->put, data,
				   MIN(size, space));
}