ssize_t zms_read_hist(struct zms_fs *fs, zms_id_t id, void *data, size_t len, uint32_t cnt)
{
	int rc;
	int prev_found = 0;
	uint64_t wlk_addr;
	uint64_t rd_addr = 0;
	uint64_t wlk_prev_addr = 0;
	uint32_t cnt_his;
	struct zms_ate wlk_ate;
#ifdef CONFIG_ZMS_DATA_CRC
	uint32_t computed_data_crc;
#endif
	if (!fs) {
		LOG_ERR("Invalid fs");
		return -EINVAL;
	}

	if (!fs->ready) {
		LOG_ERR("zms not initialized");
		return -EACCES;
	}

	cnt_his = 0U;

#ifdef CONFIG_ZMS_LOOKUP_CACHE
	wlk_addr = fs->lookup_cache[zms_lookup_cache_pos(id)];

	if (wlk_addr == ZMS_LOOKUP_CACHE_NO_ADDR) {
		rc = -ENOENT;
		goto err;
	}
#else
	wlk_addr = fs->ate_wra;
#endif

	while (cnt_his <= cnt) {
		wlk_prev_addr = wlk_addr;
		/* Search for a previous valid ATE with the same ID */
		prev_found = zms_find_ate_with_id(fs, id, wlk_addr, fs->ate_wra, &wlk_ate,
						  &wlk_prev_addr);
		if (prev_found < 0) {
			return prev_found;
		}
		if (prev_found) {
			cnt_his++;
			/* wlk_prev_addr contain the ATE address of the previous found ATE. */
			rd_addr = wlk_prev_addr;
			/*
			 * compute the previous ATE address in case we need to start
			 * the research again.
			 */
			rc = zms_compute_prev_addr(fs, &wlk_prev_addr);
			if (rc) {
				return rc;
			}
			/* wlk_addr will be the start research address in the next loop */
			wlk_addr = wlk_prev_addr;
		} else {
			break;
		}
	}

	if (((!prev_found) || (wlk_ate.id != id)) || (wlk_ate.len == 0U) || (cnt_his < cnt)) {
		return -ENOENT;
	}

	if (wlk_ate.len <= ZMS_DATA_IN_ATE_SIZE) {
		/* data is stored in the ATE */
		if (data) {
			memcpy(data, &wlk_ate.data, MIN(len, wlk_ate.len));
		}
	} else {
		rd_addr &= ADDR_SECT_MASK;
		rd_addr += wlk_ate.offset;
		/* do not read or copy data if pointer is NULL */
		if (data) {
			rc = zms_flash_rd(fs, rd_addr, data, MIN(len, wlk_ate.len));
			if (rc) {
				goto err;
			}
		}
#ifdef CONFIG_ZMS_DATA_CRC
		/* Do not compute CRC for partial reads as CRC won't match */
		if (len >= wlk_ate.len) {
			computed_data_crc = crc32_ieee(data, wlk_ate.len);
			if (computed_data_crc != wlk_ate.data_crc) {
				LOG_ERR("Invalid data CRC: ATE_CRC=0x%08X, "
					"computed_data_crc=0x%08X",
					wlk_ate.data_crc, computed_data_crc);
				return -EIO;
			}
		}
#endif
	}

	return wlk_ate.len;

err:
	return rc;
}