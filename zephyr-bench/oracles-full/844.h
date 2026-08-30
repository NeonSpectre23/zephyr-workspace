static inline uint8_t sys_rand8_get(void)
{
	uint8_t ret;

	sys_rand_get(&ret, sizeof(ret));

	return ret;
}