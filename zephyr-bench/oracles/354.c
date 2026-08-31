int32_t smf_run_state(struct smf_ctx *const ctx)
{
	struct internal_ctx *const internal = (void *)&ctx->internal;

	/* No need to continue if terminate was set */
	if (internal->terminate) {
		return ctx->terminate_val;
	}

	/* Executing a states run function could cause a transition, so clear the
	 * internal state to ensure that the transition is handled correctly.
	 */
	smf_clear_internal_state(ctx);

#ifdef CONFIG_SMF_ANCESTOR_SUPPORT
	ctx->executing = ctx->current;
	if (ctx->current->run) {
		INVOKE_ACTION_HOOK(ctx, ctx->current, SMF_ACTION_RUN);
		enum smf_state_result rc = ctx->current->run(ctx);

		if (rc == SMF_EVENT_HANDLED) {
			internal->handled = true;
		}
	}

	if (smf_execute_ancestor_run_actions(ctx)) {
		return ctx->terminate_val;
	}
#else
	if (ctx->current->run) {
		INVOKE_ACTION_HOOK(ctx, ctx->current, SMF_ACTION_RUN);
		ctx->current->run(ctx);
	}
#endif
	return 0;
}