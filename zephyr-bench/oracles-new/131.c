size_t flash_img_bytes_written(struct flash_img_context *ctx)
{
	return stream_flash_bytes_written(&ctx->stream);
}