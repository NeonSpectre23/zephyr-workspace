int ring_buf_item_put(struct ring_buf *buf, uint16_t type, uint8_t value,
		      uint32_t *data32, uint8_t size32)
{
	uint8_t *dst, *data = (uint8_t *)data32;
	struct ring_element *header;
	uint32_t space, size, partial_size, total_size;
	int err;

	space = ring_buf_space_get(buf);
	size = size32 * 4;
	if (size + sizeof(struct ring_element) > space) {
		return -EMSGSIZE;
	}

	err = ring_buf_put_claim(buf, &dst, sizeof(struct ring_element));
	__ASSERT_NO_MSG(err == sizeof(struct ring_element));

	header = (struct ring_element *)dst;
	header->type = type;
	header->length = size32;
	header->value = value;
	total_size = sizeof(struct ring_element);

	do {
		partial_size = ring_buf_put_claim(buf, &dst, size);
		if (partial_size == 0) {
			break;
		}
		memcpy(dst, data, partial_size);
		size -= partial_size;
		total_size += partial_size;
		data += partial_size;
	} while (size != 0);
	__ASSERT_NO_MSG(size == 0);

	err = ring_buf_put_finish(buf, total_size);
	__ASSERT_NO_MSG(err == 0);
	ARG_UNUSED(err);

	return 0;
}