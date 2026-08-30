void sys_trace_k_sem_give_enter(struct k_sem *sem)
{
	ctf_top_semaphore_give_enter((uint32_t)(uintptr_t)sem);
}