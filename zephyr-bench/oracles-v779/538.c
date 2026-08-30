void ztest_test_skip(void)
{
	switch (cur_phase) {
	case TEST_PHASE_SETUP:
		__ztest_set_test_result(ZTEST_RESULT_SUITE_SKIP);
		break;
	case TEST_PHASE_BEFORE:
	case TEST_PHASE_TEST:
		__ztest_set_test_result(ZTEST_RESULT_SKIP);
		test_finalize();
		break;
	default:
		PRINT_DATA(" ERROR: cannot skip in test phase '%s()', bailing\n",
			   get_friendly_phase_name(cur_phase));
		test_status = ZTEST_STATUS_CRITICAL_ERROR;
		break;
	}
}