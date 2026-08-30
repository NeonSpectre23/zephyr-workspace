int fputs(const char *ZRESTRICT s, FILE *ZRESTRICT stream)
{
	int len = strlen(s);
	int ret;

	ret = fwrite(s, 1, len, stream);

	return (len == ret) ? 0 : EOF;
}