static inline atomic_val_t atomic_inc(atomic_t *target)
{
	return atomic_add(target, 1);
}