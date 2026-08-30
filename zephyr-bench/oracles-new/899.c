void sys_trace_k_timer_start(struct k_timer *timer, k_timeout_t duration, k_timeout_t period)
{
	ctf_top_timer_start((uint32_t)(uintptr_t)timer,
			    k_ticks_to_us_floor32((uint32_t)duration.ticks),
			    k_ticks_to_us_floor32((uint32_t)period.ticks));
}