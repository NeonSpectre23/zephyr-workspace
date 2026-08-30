void ztest_verify_all_test_suites_ran(void)
{
	bool all_tests_run = true;
	struct ztest_suite_node *suite;
	struct ztest_unit_test *test;

	if (IS_ENABLED(CONFIG_ZTEST_VERIFY_RUN_ALL)) {
		for (suite = _ztest_suite_node_list_start; suite < _ztest_suite_node_list_end;
		     ++suite) {
			if (suite->stats->run_count < 1) {
				PRINT_DATA("ERROR: Test suite '%s' did not run.\n", suite->name);
				all_tests_run = false;
			}
		}

		for (test = _ztest_unit_test_list_start; test < _ztest_unit_test_list_end; ++test) {
			suite = ztest_find_test_suite(test->test_suite_name);
			if (suite == NULL) {
				PRINT_DATA("ERROR: Test '%s' assigned to test suite '%s' which "
					   "doesn't "
					   "exist\n",
					   test->name, test->test_suite_name);
				all_tests_run = false;
			}
		}

		if (!all_tests_run) {
			test_status = ZTEST_STATUS_HAS_FAILURE;
		}
	}

	for (test = _ztest_unit_test_list_start; test < _ztest_unit_test_list_end; ++test) {
		if (test->stats->fail_count + test->stats->pass_count + test->stats->skip_count !=
		    test->stats->run_count) {
			PRINT_DATA("Bad stats for %s.%s\n", test->test_suite_name, test->name);
			test_status = ZTEST_STATUS_HAS_FAILURE;
		}
	}
}