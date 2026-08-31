int shell_backend_dummy_push_input(const struct shell *sh, const char *data, size_t len)
{
	struct shell_dummy *sh_dummy = (struct shell_dummy *)sh->iface->ctx;
	size_t space;

	space = sizeof(sh_dummy->input_buf) - sh_dummy->input_len;
	if (len > space) {
		return -ENOMEM;
	}

	memcpy(sh_dummy->input_buf + sh_dummy->input_len, data, len);
	sh_dummy->input_len += len;

	return 0;
}