void shell_backend_dummy_clear_output(const struct shell *sh)
{
	struct shell_dummy *sh_dummy = (struct shell_dummy *)sh->iface->ctx;

	sh_dummy->buf[0] = '\0';
	sh_dummy->len = 0;
}