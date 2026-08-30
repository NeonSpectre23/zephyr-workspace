static inline int log_backend_count_get(void)
{
	int cnt;

	STRUCT_SECTION_COUNT(log_backend, &cnt);

	return cnt;
}