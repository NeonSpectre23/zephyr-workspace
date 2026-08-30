static inline void ring_buf_reset(struct ring_buf *buf)
{
	ring_buf_internal_reset(buf, 0);
}