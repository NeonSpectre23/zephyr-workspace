void settings_mount_fcb_backend(struct settings_fcb *cf)
{
	uint8_t rbs;

	rbs = cf->cf_fcb.f_align;

	settings_line_io_init(read_handler, write_handler, get_len_cb, rbs);
}