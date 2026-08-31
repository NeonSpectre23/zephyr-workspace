int32_t timeutil_sync_skew_to_ppb(float skew)
{
	int64_t ppb64 = (int64_t)((1.0 - (double)skew) * 1E9);
	int32_t ppb32 = (int32_t)ppb64;

	return (ppb64 == ppb32) ? ppb32 : INT32_MIN;
}