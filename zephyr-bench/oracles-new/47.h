static inline int isxdigit(int a)
{
	if (isdigit(a) != 0) {
		return 1;
	}

	/* force to lowercase */
	a |= 32;

	return (('a' <= a) && (a <= 'f'));
}