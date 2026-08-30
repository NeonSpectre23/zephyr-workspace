int vsnprintf(char *ZRESTRICT str, size_t len,
	      const char *ZRESTRICT format, va_list vargs)
{
	struct emitter p;
	int     r;
	char    dummy;

	if (len == 0) {
		str = &dummy; /* write final NUL to dummy, can't change * *s */
	}

	p.ptr = str;
	p.len = (int) len;

	r = cbvprintf(sprintf_out, (void *) (&p), format, vargs);

	*(p.ptr) = 0;
	return r;
}