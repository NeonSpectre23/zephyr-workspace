void *realloc(void *ptr, size_t size)
{
	ARG_UNUSED(ptr);
	return malloc(size);
}