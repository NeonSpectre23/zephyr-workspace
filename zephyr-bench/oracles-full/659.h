static ALWAYS_INLINE k_spinlock_key_t k_spin_lock(struct k_spinlock *l)
{
	ARG_UNUSED(l);
	k_spinlock_key_t k;

	/* Note that we need to use the underlying arch-specific lock
	 * implementation.  The "irq_lock()" API in SMP context is
	 * actually a wrapper for a global spinlock!
	 */
	k.key = arch_irq_lock();

	z_spinlock_validate_pre(l);
#ifdef CONFIG_SMP
#ifdef CONFIG_TICKET_SPINLOCKS
	/*
	 * Enqueue ourselves to the end of a spinlock waiters queue
	 * receiving a ticket
	 */
	atomic_val_t ticket = atomic_inc(&l->tail);
	/* Spin until our ticket is served */
	while (atomic_get(&l->owner) != ticket) {
		arch_spin_relax();
	}
#else
	while (!atomic_cas(&l->locked, 0, 1)) {
		do {
			arch_spin_relax();
		} while (atomic_get(&l->locked) != 0);
	}
#endif /* CONFIG_TICKET_SPINLOCKS */
#endif /* CONFIG_SMP */
	z_spinlock_validate_post(l);

	return k;
}