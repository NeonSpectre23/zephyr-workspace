void sys_trace_k_timer_init(struct k_timer *timer)
{
	ctf_top_timer_init((uint32_t)(uintptr_t)timer);
}