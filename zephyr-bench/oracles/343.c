const char *shell_backend_dummy_get_output(const struct shell *sh,
					   size_t *sizep)
{
	struct shell_dummy *sh_dummy = (struct shell_dummy *)sh->iface->ctx;

	sh_dummy->buf[sh_dummy->len] = '\0';
	*sizep = sh_dummy->len;
	sh_dummy->len = 0;

	return sh_dummy->buf;
}