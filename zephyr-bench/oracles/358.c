void smf_set_terminate(struct smf_ctx *const ctx, int32_t val)
{
	struct internal_ctx *const internal = (void *)&ctx->internal;

	internal->terminate = true;
	ctx->terminate_val = val;
}