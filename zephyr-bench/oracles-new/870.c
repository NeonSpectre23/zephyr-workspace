void sys_trace_k_sem_take_enter(struct k_sem *sem, k_timeout_t timeout)
{
	ctf_top_semaphore_take_enter((uint32_t)(uintptr_t)sem,
				     k_ticks_to_us_floor32((uint32_t)timeout.ticks));
}