osPoolId osPoolCreate(const osPoolDef_t *pool_def)
{
	if (k_is_in_isr()) {
		return NULL;
	}

	return (osPoolId)pool_def;
}