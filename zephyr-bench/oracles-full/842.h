static inline uint16_t sys_rand16_get(void)
{
	uint16_t ret;

	sys_rand_get(&ret, sizeof(ret));

	return ret;
}