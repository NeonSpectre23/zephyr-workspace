void ztress_abort(void)
{
	atomic_set(&active_cnt, 0);
}