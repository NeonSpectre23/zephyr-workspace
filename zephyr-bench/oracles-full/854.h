static inline struct sys_winstream *sys_winstream_init(void *buf, int buflen)
{
	struct sys_winstream *ws = buf, tmp = { .len = buflen - sizeof(*ws) };

	*ws = tmp;
	return ws;
}