int ring_buf_item_get(struct ring_buf *buf, uint16_t *type, uint8_t *value,
		      uint32_t *data32, uint8_t *size32)
{
	uint8_t *src, *data = (uint8_t *)data32;
	struct ring_element *header;
	uint32_t size, partial_size, total_size;
	int err;

	if (ring_buf_is_empty(buf)) {
		return -EAGAIN;
	}

	err = ring_buf_get_claim(buf, &src, sizeof(struct ring_element));
	__ASSERT_NO_MSG(err == sizeof(struct ring_element));

	header = (struct ring_element *)src;

	if (data && (header->length > *size32)) {
		*size32 = header->length;
		ring_buf_get_finish(buf, 0);
		return -EMSGSIZE;
	}

	*size32 = header->length;
	*type = header->type;
	*value = header->value;
	total_size = sizeof(struct ring_element);

	size = *size32 * 4;

	do {
		partial_size = ring_buf_get_claim(buf, &src, size);
		if (partial_size == 0) {
			break;
		}
		if (data) {
			memcpy(data, src, partial_size);
			data += partial_size;
		}
		total_size += partial_size;
		size -= partial_size;
	} while (size != 0);

	err = ring_buf_get_finish(buf, total_size);
	__ASSERT_NO_MSG(err == 0);
	ARG_UNUSED(err);

	return 0;
}