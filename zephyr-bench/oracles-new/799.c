static void start_threads(void)
{
	int ret;
	int prio;
	int policy;
	struct sched_param param;

	ARRAY_FOR_EACH(forks, i) {
		LOG_DBG("Initializing philosopher %zu", i);
		ret = pthread_create(&threads[i], NULL, philosopher, INT_TO_POINTER(i));
		if (IS_ENABLED(CONFIG_SAMPLE_ERROR_CHECKING) && ret != 0) {
			errno = ret;
			perror("pthread_create");
			__ASSERT(false, "Failed to create thread");
		}

		prio = new_prio(i);
		param.sched_priority = zephyr_to_posix_priority(prio, &policy);
		ret = pthread_setschedparam(threads[i], policy, &param);
		if (IS_ENABLED(CONFIG_SAMPLE_ERROR_CHECKING) && ret != 0) {
			errno = ret;
			perror("pthread_setschedparam");
			__ASSERT(false, "Failed to set scheduler params");
		}

		if (IS_ENABLED(CONFIG_THREAD_NAME)) {
			char tname[MAX_NAME_LEN];

			snprintf(tname, sizeof(tname), "Philosopher %zu", i);
			pthread_setname_np(threads[i], tname);
		}
	}
}