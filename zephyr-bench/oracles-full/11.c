int cbvprintf(cbprintf_cb out, void *ctx, const char *fp, va_list ap)
{
	struct cb_bits	s = {
		.f = FDEV_SETUP_STREAM(cbputc, NULL, NULL, _FDEV_SETUP_WRITE),
		.out = out,
		.ctx = ctx,
	};
	return vfprintf(&s.f, fp, ap);
}