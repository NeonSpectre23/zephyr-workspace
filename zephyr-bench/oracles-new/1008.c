int video_estimate_fmt_size(struct video_format *fmt)
{
	if (fmt == NULL) {
		return -EINVAL;
	}

	switch (fmt->pixelformat) {
	case VIDEO_PIX_FMT_JPEG:
	case VIDEO_PIX_FMT_H264:
		/* Rough estimate for the worst case (quality = 100) */
		fmt->pitch = 0;
		fmt->size = fmt->width * fmt->height * 2;
		break;
	default:
		/* Uncompressed format */
		fmt->pitch = fmt->width * video_bits_per_pixel(fmt->pixelformat) / BITS_PER_BYTE;
		if (fmt->pitch == 0) {
			return -ENOTSUP;
		}
		fmt->size = fmt->pitch * fmt->height;
		break;
	}

	return 0;
}