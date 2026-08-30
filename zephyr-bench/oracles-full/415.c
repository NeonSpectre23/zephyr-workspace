uint32_t ring_buf_put(struct ring_buf *buf, const uint8_t *data, uint32_t size)
{
	uint8_t *dst;
	uint32_t partial_size;
	uint32_t total_size = 0U;
	int err;

	do {
		partial_size = ring_buf_put_claim(buf, &dst, size);
		if (partial_size == 0) {
			break;
		}
		memcpy(dst, data, partial_size);
		total_size += partial_size;
		size -= partial_size;
		data += partial_size;
	} while (size != 0);

	err = ring_buf_put_finish(buf, total_size);
	__ASSERT_NO_MSG(err == 0);
	ARG_UNUSED(err);

	return total_size;
}