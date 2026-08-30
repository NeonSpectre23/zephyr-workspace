static int cleanup_test(struct ztest_unit_test *test)
{
	int ret = TC_PASS;
	int mock_status;

	mock_status = z_cleanup_mock();

#ifdef KERNEL
	/* we need to remove the ztest_thread information from the timeout_q.
	 * Because we reuse the same k_thread structure this would
	 * causes some problems.
	 */
	if (IS_ENABLED(CONFIG_MULTITHREADING)) {
		k_thread_abort(&ztest_thread);
	}
#endif

	if (!ret && mock_status == 1) {
		PRINT_DATA("Test %s failed: Unused mock parameter values\n", test->name);
		ret = TC_FAIL;
	} else if (!ret && mock_status == 2) {
		PRINT_DATA("Test %s failed: Unused mock return values\n", test->name);
		ret = TC_FAIL;
	} else {
		;
	}

	return ret;
}