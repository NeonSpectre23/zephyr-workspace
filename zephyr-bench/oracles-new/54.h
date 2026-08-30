static ALWAYS_INLINE void k_spin_unlock(struct k_spinlock *l,
					k_spinlock_key_t key)
{
	ARG_UNUSED(l);
#ifdef CONFIG_SPIN_VALIDATE
	__ASSERT(z_spin_unlock_valid(l), "Not my spinlock %p", l);

#if defined(CONFIG_SPIN_LOCK_TIME_LIMIT) && (CONFIG_SPIN_LOCK_TIME_LIMIT != 0)
	uint32_t delta = sys_clock_cycle_get_32() - l->lock_time;

	__ASSERT(delta < CONFIG_SPIN_LOCK_TIME_LIMIT,
		 "Spin lock %p held %u cycles, longer than limit of %u cycles",
		 l, delta, CONFIG_SPIN_LOCK_TIME_LIMIT);
#endif /* CONFIG_SPIN_LOCK_TIME_LIMIT */
#endif /* CONFIG_SPIN_VALIDATE */

#ifdef CONFIG_SMP
#ifdef CONFIG_TICKET_SPINLOCKS
	/* Give the spinlock to the next CPU in a FIFO */
	(void)atomic_inc(&l->owner);
#else
	/* Strictly we don't need atomic_clear() here (which is an
	 * exchange operation that returns the old value).  We are always
	 * setting a zero and (because we hold the lock) know the existing
	 * state won't change due to a race.  But some architectures need
	 * a memory barrier when used like this, and we don't have a
	 * Zephyr framework for that.
	 */
	(void)atomic_clear(&l->locked);
#endif /* CONFIG_TICKET_SPINLOCKS */
#endif /* CONFIG_SMP */
	arch_irq_unlock(key.key);
}