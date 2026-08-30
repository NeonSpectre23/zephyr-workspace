size_t fwrite(const void *ZRESTRICT ptr, size_t size, size_t nitems,
			  FILE *ZRESTRICT stream)
{
	return zephyr_fwrite(ptr, size, nitems, stream);
}