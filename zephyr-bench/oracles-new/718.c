void settings_mount_file_backend(struct settings_file *cf)
{
	settings_line_io_init(read_handler, write_handler, get_len_cb, 1);
}