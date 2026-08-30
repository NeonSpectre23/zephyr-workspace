int vsprintf(char *ZRESTRICT str, const char *ZRESTRICT format,
	     va_list vargs)
{
	struct emitter p;
	int     r;

	p.ptr = str;
	p.len = (int) 0x7fffffff; /* allow up to "maxint" characters */

	r = cbvprintf(sprintf_out, (void *) (&p), format, vargs);

	*(p.ptr) = 0;
	return r;
}