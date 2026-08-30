static inline int cbprintf_fsc_package(void *in_packaged,
				       size_t in_len,
				       void *packaged,
				       size_t len)
{
	return cbprintf_package_copy(in_packaged, in_len, packaged, len,
				     CBPRINTF_PACKAGE_CONVERT_RO_STR |
				     CBPRINTF_PACKAGE_CONVERT_RW_STR, NULL, 0);
}