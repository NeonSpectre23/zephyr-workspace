void smf_set_initial(struct smf_ctx *const ctx, const struct smf_state *init_state)
{
#ifdef CONFIG_SMF_INITIAL_TRANSITION
	/*
	 * The final target will be the deepest leaf state that
	 * the target contains. Set that as the real target.
	 */
	while (init_state->initial != NULL) {
		init_state = init_state->initial;
	}
#endif

	smf_clear_internal_state(ctx);
	ctx->current = init_state;
	ctx->previous = NULL;
	ctx->terminate_val = 0;

#ifdef CONFIG_SMF_INSTRUMENTATION
	ctx->hooks = NULL;
#endif /* CONFIG_SMF_INSTRUMENTATION */

#ifdef CONFIG_SMF_ANCESTOR_SUPPORT
	struct internal_ctx *const internal = (void *)&ctx->internal;

	ctx->executing = init_state;
	/* topmost is the root ancestor of init_state, its parent == NULL */
	const struct smf_state *topmost = get_child_of(init_state, NULL);

	/* Execute topmost state entry action, since smf_execute_all_entry_actions()
	 * doesn't
	 */
	if (topmost->entry) {
		ctx->executing = topmost;
		INVOKE_ACTION_HOOK(ctx, topmost, SMF_ACTION_ENTRY);
		topmost->entry(ctx);
		ctx->executing = init_state;
		if (internal->terminate) {
			/* No need to continue if terminate was set */
			return;
		}
	}

	if (smf_execute_all_entry_actions(ctx, init_state, topmost)) {
		/* No need to continue if terminate was set */
		return;
	}
#else
	/* execute entry action if it exists */
	if (init_state->entry) {
		INVOKE_ACTION_HOOK(ctx, init_state, SMF_ACTION_ENTRY);
		init_state->entry(ctx);
	}
#endif
}