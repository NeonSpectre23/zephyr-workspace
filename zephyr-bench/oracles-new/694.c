static int run_test(struct ztest_suite_node *suite, struct ztest_unit_test *test, void *data)
{
	int ret = TC_PASS;

#if CONFIG_ZTEST_TEST_DELAY_MS > 0
	k_busy_wait(CONFIG_ZTEST_TEST_DELAY_MS * USEC_PER_MSEC);
#endif
	TC_START(test->name);

	__ztest_set_test_phase(TEST_PHASE_BEFORE);

	/* If the suite's setup function marked us as skipped, don't bother
	 * running the tests.
	 */
	if (IS_ENABLED(CONFIG_MULTITHREADING)) {
		get_start_time_cyc();
		k_thread_create(&ztest_thread, ztest_thread_stack,
				K_THREAD_STACK_SIZEOF(ztest_thread_stack), test_cb, suite, test,
				data, CONFIG_ZTEST_THREAD_PRIORITY, K_INHERIT_PERMS, K_FOREVER);

		k_thread_access_grant(&ztest_thread, suite, test, suite->stats);
		if (test->name != NULL) {
			k_thread_name_set(&ztest_thread, test->name);
		}
		/* Only start the thread if we're not skipping the suite */
		if (test_result != ZTEST_RESULT_SUITE_SKIP &&
		    test_result != ZTEST_RESULT_SUITE_FAIL) {
			k_thread_start(&ztest_thread);
			k_thread_join(&ztest_thread, K_FOREVER);
		}
	} else if (test_result != ZTEST_RESULT_SUITE_SKIP &&
		   test_result != ZTEST_RESULT_SUITE_FAIL) {
		__ztest_set_test_result(ZTEST_RESULT_PENDING);
		get_start_time_cyc();
		run_test_rules(/*is_before=*/true, test, data);
		if (suite->before) {
			suite->before(data);
		}
		run_test_functions(suite, test, data);
	}

	__ztest_set_test_phase(TEST_PHASE_AFTER);
	if (suite->after != NULL) {
		suite->after(data);
	}
	run_test_rules(/*is_before=*/false, test, data);

	get_test_duration_ms();
	if (tc_spend_time > test->stats->duration_worst_ms) {
		test->stats->duration_worst_ms = tc_spend_time;
	}

	__ztest_set_test_phase(TEST_PHASE_FRAMEWORK);

	/* Flush all logs in case deferred mode and default logging thread are used. */
	while (IS_ENABLED(CONFIG_TEST_LOGGING_FLUSH_AFTER_TEST) &&
	       IS_ENABLED(CONFIG_LOG_PROCESS_THREAD) && log_data_pending()) {
		k_msleep(100);
	}

	if (test_result == ZTEST_RESULT_FAIL || test_result == ZTEST_RESULT_SUITE_FAIL ||
	    failed_expectation) {
		ret = TC_FAIL;
		failed_expectation = false;
	} else if (test_result == ZTEST_RESULT_SKIP || test_result == ZTEST_RESULT_SUITE_SKIP) {
		ret = TC_SKIP;
	}

	if (test_result == ZTEST_RESULT_PASS || !FAIL_FAST) {
		ret |= cleanup_test(test);
	}

	ret = get_final_test_result(test, ret);
	Z_TC_END_RESULT(ret, test->name);
	if (ret == TC_SKIP && current_test_failed_assumption) {
		test_status = ZTEST_STATUS_HAS_FAILURE;
	}

	return ret;
}