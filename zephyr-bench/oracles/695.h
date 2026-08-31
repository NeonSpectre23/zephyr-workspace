static inline int ubx_frame_encode(uint8_t class, uint8_t id,
				    const uint8_t *payload, size_t payload_len,
				    uint8_t *buf, size_t buf_len)
{
	if (buf_len < UBX_FRAME_SZ(payload_len)) {
		return -EINVAL;
	}

	struct ubx_frame *frame = (struct ubx_frame *)buf;

	frame->preamble_sync_char_1 = UBX_PREAMBLE_SYNC_CHAR_1;
	frame->preamble_sync_char_2 = UBX_PREAMBLE_SYNC_CHAR_2;
	frame->class = class;
	frame->id = id;
	frame->payload_size = payload_len;
	memcpy(frame->payload_and_checksum, payload, payload_len);

	uint16_t checksum = ubx_calc_checksum(frame, UBX_FRAME_SZ(payload_len));

	frame->payload_and_checksum[payload_len] = checksum & 0xFF;
	frame->payload_and_checksum[payload_len + 1] = (checksum >> 8) & 0xFF;

	return UBX_FRAME_SZ(payload_len);
}