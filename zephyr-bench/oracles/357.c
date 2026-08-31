void smf_set_state(struct smf_ctx *const ctx, const struct smf_state *new_state)
{
	struct internal_ctx *const internal = (void *)&ctx->internal;

	if (new_state == NULL) {
		LOG_ERR("new_state cannot be NULL");
		INVOKE_ERROR_HOOK(ctx, SMF_ERR_NULL_TRANSITION);
		return;
	}

	/*
	 * It does not make sense to call smf_set_state in an exit phase of a state
	 * since we are already in a transition; we would always ignore the
	 * intended state to transition into.
	 */
	if (internal->is_exit) {
		LOG_ERR("Calling %s from exit action", __func__);
		INVOKE_ERROR_HOOK(ctx, SMF_ERR_TRANSITION_IN_EXIT);
		return;
	}

#ifdef CONFIG_SMF_ANCESTOR_SUPPORT
	const struct smf_state *topmost;

	if (ctx->executing != new_state && ctx->executing->parent == new_state->parent) {
		/* Optimize sibling transitions (different states under same parent) */
		topmost = ctx->executing->parent;
	} else if (is_descendant_of(ctx->executing, new_state)) {
		/* new state is a parent of where we are now*/
		topmost = new_state;
	} else if (is_descendant_of(new_state, ctx->executing)) {
		/* we are a parent of the new state */
		topmost = ctx->executing;
	} else {
		/* not directly related, find LCA */
		topmost = get_lca_of(ctx->executing, new_state);
	}

	internal->is_exit = true;
	internal->new_state = true;

	/* call all exit actions up to (but not including) the topmost */
	if (smf_execute_all_exit_actions(ctx, topmost)) {
		/* No need to continue if terminate was set in the exit action */
		return;
	}

	/* if self-transition, call the exit action */
	if ((ctx->executing == new_state) && (new_state->exit)) {
		INVOKE_ACTION_HOOK(ctx, new_state, SMF_ACTION_EXIT);
		new_state->exit(ctx);

		/* No need to continue if terminate was set in the exit action */
		if (internal->terminate) {
			return;
		}
	}

	internal->is_exit = false;

	/* if self transition, call the entry action */
	if ((ctx->executing == new_state) && (new_state->entry)) {
		INVOKE_ACTION_HOOK(ctx, new_state, SMF_ACTION_ENTRY);
		new_state->entry(ctx);

		/* No need to continue if terminate was set in the entry action */
		if (internal->terminate) {
			return;
		}
	}
#ifdef CONFIG_SMF_INITIAL_TRANSITION
	/*
	 * The final target will be the deepest leaf state that
	 * the target contains. Set that as the real target.
	 */
	while (new_state->initial != NULL) {
		new_state = new_state->initial;
	}
#endif

	/* update the state variables */
	ctx->previous = ctx->current;
	ctx->current = new_state;
	ctx->executing = new_state;

	INVOKE_TRANSITION_HOOK(ctx, ctx->previous, ctx->current);

	/* call all entry actions (except those of topmost) */
	if (smf_execute_all_entry_actions(ctx, new_state, topmost)) {
		/* No need to continue if terminate was set in the entry action */
		return;
	}
#else
	/* Flat state machines have a very simple transition: */
	if (ctx->current->exit) {
		internal->is_exit = true;
		INVOKE_ACTION_HOOK(ctx, ctx->current, SMF_ACTION_EXIT);
		ctx->current->exit(ctx);
		/* No need to continue if terminate was set in the exit action */
		if (internal->terminate) {
			return;
		}
		internal->is_exit = false;
	}
	/* update the state variables */
	ctx->previous = ctx->current;
	ctx->current = new_state;

	INVOKE_TRANSITION_HOOK(ctx, ctx->previous, ctx->current);

	if (ctx->current->entry) {
		INVOKE_ACTION_HOOK(ctx, ctx->current, SMF_ACTION_ENTRY);
		ctx->current->entry(ctx);
		/* No need to continue if terminate was set in the entry action */
		if (internal->terminate) {
			return;
		}
	}
#endif
}