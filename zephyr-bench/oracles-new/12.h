static inline int cbprintf_package_copy(void *in_packaged,
					size_t in_len,
					void *packaged,
					size_t len,
					uint32_t flags,
					uint16_t *strl,
					size_t strl_len)
{
	struct z_cbprintf_buf_desc buf_desc = {
		.buf = packaged,
		.size = len,
		.off = 0,
	};

	return cbprintf_package_convert(in_packaged, in_len,
					packaged ? z_cbprintf_cpy : NULL, &buf_desc,
					flags, strl, strl_len);
}