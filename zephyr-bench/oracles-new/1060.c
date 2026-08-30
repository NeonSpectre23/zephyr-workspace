ssize_t zms_calc_free_space(struct zms_fs *fs)
{
	int rc;
	int prev_found = 0;
	int sec_closed;
	struct zms_ate step_ate;
	struct zms_ate wlk_ate;
	struct zms_ate empty_ate;
	struct zms_ate close_ate;
	uint64_t step_addr;
	uint64_t step_prev_addr;
	uint64_t close_addr;
	uint32_t remaining_sectors;
	uint32_t data_wra;
	uint32_t ate_wra;
	uint8_t current_cycle;
	ssize_t free_space = 0;

	if (!fs) {
		LOG_ERR("Invalid fs");
		return -EINVAL;
	}

	if (!fs->ready) {
		LOG_ERR("zms not initialized");
		return -EACCES;
	}

	step_addr = fs->ate_wra;
	current_cycle = fs->sector_cycle;
	/* there is always one reserved sector for garbage collection */
	remaining_sectors = fs->sector_count - 1;

	while (1) {
		/* Count entries in the current sector as if it were garbage-collected.
		 * Initialize data_wra and ate_wra as they would be in an empty sector
		 * (with bottom space reserved for empty ATE, close ATE, and GC_done ATE).
		 */
		data_wra = 0;
		ate_wra = fs->sector_size - 4 * fs->ate_size;

		for (close_addr = zms_close_ate_addr(fs, step_addr); step_addr < close_addr;
		     step_addr += fs->ate_size) {
			rc = zms_flash_ate_rd(fs, step_addr, &step_ate);
			if (rc) {
				return rc;
			}

			/* Invalid and deleted ATEs are free spaces.
			 * Header ATEs are already retrieved from free space
			 */
			if (!zms_ate_valid_different_sector(fs, &step_ate, current_cycle) ||
			    (step_ate.id == ZMS_HEAD_ID) || (step_ate.len == 0)) {
				continue;
			}

			/* Search for a more recent, valid ATE with the same ID */
			prev_found = zms_find_ate_with_id(fs, step_ate.id, fs->ate_wra, step_addr,
							  &wlk_ate, &step_prev_addr);
			if (prev_found < 0) {
				return prev_found;
			}

			if (!prev_found) {
				/* this item would not have been garbage collected */
				if (step_ate.len > ZMS_DATA_IN_ATE_SIZE) {
					data_wra += zms_al_size(fs, step_ate.len);
				}
				ate_wra -= fs->ate_size;
			}
		}

		/* reached end of sector */
		free_space += zms_free_space(fs, data_wra, ate_wra);

		remaining_sectors--;
		if (remaining_sectors == 0) {
			/* explored all sectors */
			return free_space;
		}

		/* jump to previous sector */
		if (SECTOR_NUM(step_addr) == 0U) {
			step_addr += ((uint64_t)(fs->sector_count - 1) << ADDR_SECT_SHIFT);
		} else {
			step_addr -= (1ULL << ADDR_SECT_SHIFT);
		}

		/* verify if the sector is closed */
		sec_closed = zms_validate_closed_sector(fs, step_addr, &empty_ate, &close_ate);
		if (sec_closed < 0) {
			return sec_closed;
		}

		/* If closed, then update step_addr to point to the last ATE in this sector.
		 * Otherwise, this sector is empty and step_addr points to its close ATE.
		 */
		if (sec_closed == 1) {
			step_addr &= ADDR_SECT_MASK;
			step_addr += close_ate.offset;
			/* When changing the sector let's get the new cycle counter */
			current_cycle = close_ate.cycle_cnt;
		}
	}
}