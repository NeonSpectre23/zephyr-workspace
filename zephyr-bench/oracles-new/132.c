int flash_img_check(struct flash_img_context *ctx,
		    const struct flash_img_check *fic,
		    uint8_t area_id)
{
	struct flash_area_check fac;
	int rc;

	if (!ctx || !fic) {
		return -EINVAL;
	}

	rc = flash_area_open(area_id,
			     (const struct flash_area **)&(ctx->flash_area));
	if (rc) {
		return rc;
	}

	fac.match = fic->match;
	fac.clen = fic->clen;
	fac.off = boot_get_image_start_offset(area_id);
	fac.rbuf = ctx->buf;
	fac.rblen = sizeof(ctx->buf);

	rc = flash_area_check_int_sha256(ctx->flash_area, &fac);

	flash_area_close(ctx->flash_area);
	ctx->flash_area = NULL;

	return rc;
}