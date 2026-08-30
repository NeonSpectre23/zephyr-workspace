void sys_trace_k_sem_init(struct k_sem *sem, int ret)
{
	ctf_top_semaphore_init((uint32_t)(uintptr_t)sem, (int32_t)ret);
}