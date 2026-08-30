void shell_backend_dummy_clear_input(const struct shell *sh)
{
	struct shell_dummy *sh_dummy = (struct shell_dummy *)sh->iface->ctx;

	sh_dummy->input_len = 0;
	sh_dummy->input_pos = 0;
}