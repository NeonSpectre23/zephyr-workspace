__weak z_early_rand_get(uint8_t *buf, size_t length)
{
	static uint64_t state = (uint64_t)CONFIG_TIMER_RANDOM_INITIAL_STATE;
	int rc;

#ifdef CONFIG_ENTROPY_HAS_DRIVER
	const struct device *const entropy = DEVICE_DT_GET_OR_NULL(DT_CHOSEN(zephyr_entropy));

	if ((entropy != NULL) && device_is_ready(entropy)) {
		/* Try to see if driver provides an ISR-specific API */
		rc = entropy_get_entropy_isr(entropy, buf, length, ENTROPY_BUSYWAIT);
		if (rc > 0) {
			length -= rc;
			buf += rc;
		}
	}
#endif /* CONFIG_ENTROPY_HAS_DRIVER */

	while (length > 0) {
		uint32_t val;

		state = state + k_cycle_get_32();
		state = state * 2862933555777941757ULL + 3037000493ULL;
		val = (uint32_t)(state >> 32);
		rc = min(length, sizeof(val));
		arch_early_memcpy((void *)buf, &val, rc);

		length -= rc;
		buf += rc;
	}
}