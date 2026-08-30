void ztest_test_pass(void)
{
	switch (cur_phase) {
	case TEST_PHASE_TEST:
		__ztest_set_test_result(ZTEST_RESULT_PASS);
		test_finalize();
		break;
	default:
		PRINT_DATA(" ERROR: cannot pass in test phase '%s()', bailing\n",
			   get_friendly_phase_name(cur_phase));
		test_status = ZTEST_STATUS_CRITICAL_ERROR;
		if (cur_phase == TEST_PHASE_BEFORE) {
			test_finalize();
		}
		break;
	}
}