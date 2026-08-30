void ztest_run_all(const void *state, bool shuffle, int suite_iter, int case_iter)
{
	ztest_api.run_all(state, shuffle, suite_iter, case_iter);
}