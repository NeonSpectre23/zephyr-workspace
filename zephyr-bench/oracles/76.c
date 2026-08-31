int gnss_rtk_decoder_frame_get(uint8_t *buf, size_t buf_len,
			       uint8_t **data, size_t *data_len)
{
	for (size_t i = 0 ; (i + RTCM3_FRAME_OVERHEAD - 1) < buf_len ; i++) {
		if (buf[i] != RTCM3_SYNC_BYTE) {
			continue;
		}

		struct rtcm3_frame *frame = (struct rtcm3_frame *)&buf[i];
		uint16_t payload_len = RTCM3_FRAME_PAYLOAD_SZ(frame->hdr);
		uint16_t remaining_bytes = buf_len - i;

		if (payload_len == 0 ||
		    RTCM3_FRAME_SZ(payload_len) > remaining_bytes) {
			continue;
		}

		if (crc24q_rtcm3((const uint8_t *)frame,
				 RTCM3_FRAME_SZ(payload_len)) == 0) {
			*data = (uint8_t *)frame;
			*data_len = RTCM3_FRAME_SZ(payload_len);

			return 0;
		}
	}

	return -ENOENT;
}