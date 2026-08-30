static int rand_get(uint8_t *dst, size_t outlen, bool csrand)
{
	uint32_t random_num;
	int ret;

	if (!device_is_ready(entropy_dev)) {
		return -ENODEV;
	}

	ret = entropy_get_entropy(entropy_dev, dst, outlen);

	if (unlikely(ret < 0)) {
		/* Don't try to fill the buffer in case of
		 * cryptographically secure random numbers, just
		 * propagate the driver error.
		 */
		if (csrand) {
			return ret;
		}

		/* Use system timer in case the entropy device couldn't deliver
		 * 32-bit of data.  There's not much that can be done in this
		 * situation.  An __ASSERT() isn't used here as the HWRNG might
		 * still be gathering entropy during early boot situations.
		 */

		uint32_t len = 0;
		uint32_t blocksize = 4;

		while (len < outlen) {
			size_t copylen = outlen - len;

			if (copylen > blocksize) {
				copylen = blocksize;
			}

			random_num = k_cycle_get_32();
			(void)memcpy(&(dst[len]), &random_num, copylen);
			len += copylen;
		}
	}

	return 0;
}