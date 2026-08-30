int cfb_invert_area(const struct device *dev, int16_t x, int16_t y,
		    uint16_t width, uint16_t height)
{
	const struct char_framebuffer *fb = &char_fb;
	const bool need_reverse = ((fb->screen_info & SCREEN_INFO_MONO_MSB_FIRST) != 0);

	if ((x + width) < 0 || x >= fb->x_res) {
		return 0;
	}

	if ((y + height) < 0 || y >= fb->y_res) {
		return 0;
	}

	if (x < 0) {
		width += x;
		x = 0;
	}

	if (y < 0) {
		height += y;
		y = 0;
	}

	if (width > (fb->x_res - x)) {
		width = fb->x_res - x;
	}

	if (height > (fb->y_res - y)) {
		height = fb->y_res - y;
	}


	if ((fb->screen_info & SCREEN_INFO_MONO_VTILED)) {
		for (size_t i = x; i < (x + width); i++) {
			for (size_t j = y; j < (y + height); j++) {
				/*
				 * Process inversion in the y direction
				 * by separating per 8-line boundaries.
				 */

				const size_t index = ((j / 8) * fb->x_res) + i;
				const uint8_t remains = y + height - j;

				/*
				 * Make mask to prevent overwriting the drawing contents that on
				 * between the start line or end line and the 8-line boundary.
				 */
				if ((j % 8) > 0) {
					uint8_t m = BIT_MASK((j % 8));
					uint8_t b = fb->buf[index];

					/*
					 * Generate mask for remaining lines in case of
					 * drawing within 8 lines from the start line
					 */
					if (remains < 8) {
						m |= BIT_MASK((8 - (j % 8) + remains))
						     << ((j % 8) + remains);
					}

					if (need_reverse) {
						m = byte_reverse(m);
					}

					fb->buf[index] = (b ^ (~m));
					j += 7 - (j % 8);
				} else if (remains >= 8) {
					/* No mask required if no start or end line is included */
					fb->buf[index] = ~fb->buf[index];
					j += 7;
				} else {
					uint8_t m = BIT_MASK(8 - remains) << (remains);
					uint8_t b = fb->buf[index];

					if (need_reverse) {
						m = byte_reverse(m);
					}

					fb->buf[index] = (b ^ (~m));
					j += (remains - 1);
				}
			}
		}
	} else {
		const size_t bytes_per_row = fb->x_res / 8U;

		for (uint16_t j = y; j < (y + height); j++) {
			const uint16_t start_byte = x / 8U;
			const uint16_t end_byte = (x + width - 1U) / 8U;

			for (uint16_t b = start_byte; b <= end_byte; b++) {
				const size_t index = j * bytes_per_row + b;
				const uint8_t bit_start = (b == start_byte) ? (x % 8U) : 0U;
				const uint8_t bit_end =
					(b == end_byte) ? ((x + width - 1U) % 8U) : 7U;
				uint8_t m;

				if (bit_end >= bit_start) {
					m = BIT_MASK(bit_end - bit_start + 1U) << bit_start;
				} else {
					m = 0U;
				}

				if (need_reverse) {
					m = byte_reverse(m);
				}

				/* invert byte with computed mask */
				fb->buf[index] ^= m;
			}
		}
	}

	return 0;
}