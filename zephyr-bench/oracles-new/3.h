static inline bool atomic_cas(atomic_t *target, atomic_val_t old_value,
			  atomic_val_t new_value)
{
	return __atomic_compare_exchange_n(target, &old_value, new_value,
					   0, __ATOMIC_SEQ_CST,
					   __ATOMIC_SEQ_CST);
}