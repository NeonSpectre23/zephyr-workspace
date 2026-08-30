int write_log_to_file(uint8_t *data, size_t length, void *ctx)
{
	int rc;
	struct fs_file_t *f = &fs_file;

	if (backend_state == BACKEND_FS_NOT_INITIALIZED) {
		if (check_log_volume_available()) {
			return length;
		}
		rc = create_log_dir(CONFIG_LOG_BACKEND_FS_DIR);
		if (!rc) {
			rc = allocate_new_file(&fs_file);
		}
		backend_state = (rc ? BACKEND_FS_CORRUPTED : BACKEND_FS_OK);
	}

	if (backend_state == BACKEND_FS_OK) {

		/* Check if new data overwrites max file size.
		 * If so, create new log file.
		 */
		int size = fs_tell(f);

		if (size < 0) {
			backend_state = BACKEND_FS_CORRUPTED;

			return length;
		} else if ((size + length) > CONFIG_LOG_BACKEND_FS_FILE_SIZE) {
			rc = allocate_new_file(f);

			if (rc < 0) {
				goto on_error;
			}
		}

		rc = fs_write(f, data, length);
		if (rc >= 0) {
			if (IS_ENABLED(CONFIG_LOG_BACKEND_FS_OVERWRITE) &&
			    (rc != length)) {
				del_oldest_log();

				return 0;
			}
			/* If overwrite is disabled, full memory
			 * cause the log record abandonment.
			 */
			length = rc;
		} else {
			rc = check_log_file_exist(newest);
			if (rc == 0) {
				/* file was lost somehow
				 * try to get a new one
				 */
				file_ctr--;
				rc = allocate_new_file(f);
				if (rc < 0) {
					goto on_error;
				}
			} else if (rc < 0) {
				/* fs is corrupted*/
				goto on_error;
			}
			length = 0;
		}
	}

	return length;

on_error:
	backend_state = BACKEND_FS_CORRUPTED;
	return length;
}