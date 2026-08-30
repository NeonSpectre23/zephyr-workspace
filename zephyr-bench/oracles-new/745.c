int shell_init(const struct shell *sh, const void *transport_config,
	       struct shell_backend_config_flags cfg_flags,
	       bool log_backend, uint32_t init_log_level)
{
	__ASSERT_NO_MSG(sh);
	__ASSERT_NO_MSG(sh->ctx && sh->iface && sh->default_prompt);

	if (sh->ctx->tid) {
		return -EALREADY;
	}

	int err = instance_init(sh, transport_config, cfg_flags);

	if (err != 0) {
		return err;
	}

	k_tid_t tid = k_thread_create(sh->thread,
				  sh->stack, CONFIG_SHELL_STACK_SIZE,
				  shell_thread, (void *)sh, (void *)log_backend,
				  UINT_TO_POINTER(init_log_level),
				  SHELL_THREAD_PRIORITY, 0, K_NO_WAIT);

	sh->ctx->tid = tid;
	k_thread_name_set(tid, sh->name);

	return 0;
}

