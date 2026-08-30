static void start_threads(void)
{
	osThreadId id;

	for (int i = 0; i < NUM_PHIL; i++) {
		int prio = new_prio(i);
		id = osThreadCreate(osThread(philosopher), INT_TO_POINTER(i));
		osThreadSetPriority(id, prio);
	}
}