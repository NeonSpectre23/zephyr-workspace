bool is_tracing_enabled(void)
{
	return atomic_get(&tracing_state) == TRACING_ENABLE;
}