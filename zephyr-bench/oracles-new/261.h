static inline uint32_t sys_rand32_get(void)
{
	uint32_t ret;

	sys_rand_get(&ret, sizeof(ret));

	return ret;
}