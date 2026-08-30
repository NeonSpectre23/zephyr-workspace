int rand_r(unsigned int *seed)
{
	*seed = (MULTIPLIER * *seed + INCREMENT) & OUTPUT_BITS;

	return *seed;
}