int mipi_stp_decoder_decode(const uint8_t *data, size_t len)
{
	static enum stp_id curr_id = STP_INVALID;
	static uint8_t data_buf[8] __aligned(sizeof(uint64_t));
	static uint8_t ts_buf[8] __aligned(sizeof(uint64_t));
	uint64_t *data64 = (uint64_t *)data_buf;
	uint64_t *ts64 = (uint64_t *)ts_buf;
	size_t nlen = 2 * len;

	do {
		switch (state) {
		case STP_STATE_OUT_OF_SYNC: {
			uint8_t b = get_nibble(data, noff);

			noff++;
			if (ncnt < 21 && b == 0xF) {
				ncnt++;
			} else if (ncnt == 21 && b == 0) {
				curr_id = STP_INVALID;
				ncnt = 0;

				items[STP_ASYNC].cb(0, 0);
				state = STP_STATE_OP;
			} else {
				ncnt = 0;
			}
			break;
		}
		case STP_STATE_OP: {
			curr_id = get_op(data, &noff, &nlen, &ncnt, &ntotal);
			if (curr_id != STP_INVALID) {
				ntotal = items[curr_id].d_ncnt;
				ncnt = 0;
				if (ntotal > 0) {
					state = STP_STATE_DATA;
					data64[0] = 0;
				} else if (items[curr_id].has_ts) {
					state = STP_STATE_TS;
				} else {
					/* item without data and ts, notify. */
					items[curr_id].cb(0, 0);
					curr_id = STP_INVALID;
				}
			}
			break;
		}
		case STP_STATE_DATA: {
			size_t ncpy = MIN(ntotal - ncnt, nlen - noff);

			get_nibbles(data, noff, data_buf, ncnt, ncpy);

			ncnt += ncpy;
			noff += ncpy;
			if (ncnt == ntotal) {
				swap_n(data_buf, ntotal);
				ncnt = 0;
				if (items[curr_id].has_ts) {
					ncnt = 0;
					ntotal = 0;
					state = STP_STATE_TS;
				} else {
					items[curr_id].cb(*data64, 0);
					curr_id = STP_INVALID;
					state = STP_STATE_OP;
					ntotal = 0;
					ncnt = 0;
				}
			}
			break;
		}
		case STP_STATE_TS:
			if (ntotal == 0 && ncnt == 0) {
				/* TS to be read but length is unknown yet */
				*ts64 = 0;
				ntotal = get_nibble(data, noff);
				noff++;
				/* Values up to 12 represents number of nibbles on which
				 * timestamp is encoded. Above are the exceptions:
				 * - 13 => 14 nibbles
				 * - 14 => 16 nibbles
				 */
				if (ntotal > 12) {
					if (ntotal == 13) {
						ntotal = 14;
						base_ts = ~BIT64_MASK(4 * ntotal) & prev_ts;
					} else {
						ntotal = 16;
						base_ts = 0;
					}
				} else {
					base_ts = ~BIT64_MASK(4 * ntotal) & prev_ts;
				}

			} else {
				size_t ncpy = MIN(ntotal - ncnt, nlen - noff);

				get_nibbles(data, noff, ts_buf, ncnt, ncpy);
				ncnt += ncpy;
				noff += ncpy;
				if (ncnt == ntotal) {
					swap_n(ts_buf, ntotal);
					prev_ts = base_ts | *ts64;
					items[curr_id].cb(*data64, prev_ts);
					curr_id = STP_INVALID;
					state = STP_STATE_OP;
					ntotal = 0;
					ncnt = 0;
				}
			}
			break;

		default:
			break;
		}
	} while (noff < nlen);

	noff = 0;

	return 0;
}