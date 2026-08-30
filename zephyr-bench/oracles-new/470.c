static void notify(const struct log_backend *const backend, enum log_backend_evt event,
		   union log_backend_evt_arg *arg)
{
	if (event == LOG_BACKEND_EVT_PROCESS_THREAD_DONE) {
		if (backend_state == BACKEND_FS_OK) {
			int rc = fs_sync(&fs_file);

			if (rc != 0) {
				backend_state = BACKEND_FS_CORRUPTED;
			}
		}
	}
}