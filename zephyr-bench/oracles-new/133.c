int flash_img_init(struct flash_img_context *ctx)
{
	return flash_img_init_id(ctx, UPLOAD_FLASH_AREA_ID);
}