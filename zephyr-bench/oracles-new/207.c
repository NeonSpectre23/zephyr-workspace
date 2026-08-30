bool k_can_yield(void)
{
	unsigned int k = arch_irq_lock();
	bool irq_locked = !arch_irq_unlocked(k);

	arch_irq_unlock(k);
	return !(k_is_pre_kernel() || k_is_in_isr() || irq_locked ||
		 z_is_idle_thread_object(_current));
}