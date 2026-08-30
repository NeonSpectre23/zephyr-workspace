static inline int shell_backend_count_get(void)
{
	int cnt;

	STRUCT_SECTION_COUNT(shell, &cnt);

	return cnt;
}