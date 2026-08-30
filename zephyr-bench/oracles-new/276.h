static inline int timespec_compare(const struct timespec *a, const struct timespec *b)
{
	__ASSERT_NO_MSG((a != NULL) && timespec_is_valid(a));
	__ASSERT_NO_MSG((b != NULL) && timespec_is_valid(b));

	return (((a->tv_sec == b->tv_sec) && (a->tv_nsec < b->tv_nsec)) * -1) +
	       (((a->tv_sec == b->tv_sec) && (a->tv_nsec > b->tv_nsec)) * 1) +
	       ((a->tv_sec < b->tv_sec) * -1) + ((a->tv_sec > b->tv_sec));
}