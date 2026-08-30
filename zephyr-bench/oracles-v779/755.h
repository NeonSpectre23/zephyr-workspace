static inline bool timespec_sub(struct timespec *a, const struct timespec *b)
{
	__ASSERT_NO_MSG(a != NULL);
	__ASSERT_NO_MSG(b != NULL);

	struct timespec neg = *b;

	return timespec_negate(&neg) && timespec_add(a, &neg);
}