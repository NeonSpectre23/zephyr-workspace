static inline atomic_val_t atomic_set(atomic_t *target, atomic_val_t value)
{
	/* This builtin, as described by Intel, is not a traditional
	 * test-and-set operation, but rather an atomic exchange operation. It
	 * writes value into *ptr, and returns the previous contents of *ptr.
	 */
	return __atomic_exchange_n(target, value, __ATOMIC_SEQ_CST);
}