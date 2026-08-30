FUNC_NORETURN void thrd_exit(int res)
{
	pthread_exit(INT_TO_POINTER(res));

	CODE_UNREACHABLE;
}