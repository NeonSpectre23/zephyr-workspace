static inline bool timespec_equal(const struct timespec *a, const struct timespec *b)
{
	__ASSERT_NO_MSG(a != NULL);
	__ASSERT_NO_MSG(b != NULL);

	return (a->tv_sec == b->tv_sec) && (a->tv_nsec == b->tv_nsec);
}