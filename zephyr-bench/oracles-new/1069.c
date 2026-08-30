ssize_t zms_write(struct zms_fs *fs, zms_id_t id, const void *data, size_t len)
{
	int rc;
	size_t data_size;
	uint32_t gc_count;
	uint32_t required_space = 0U; /* no space, appropriate for delete ate */

	if (!fs) {
		LOG_ERR("Invalid fs");
		return -EINVAL;
	}

	if (!fs->ready) {
		LOG_ERR("zms not initialized");
		return -EACCES;
	}

	data_size = zms_al_size(fs, len);

	/* The maximum data size is sector size - 5 ate
	 * where: 1 ate for data, 1 ate for sector close, 1 ate for empty,
	 * 1 ate for gc done, and 1 ate to always allow a delete.
	 * We cannot also store more than 64 KB of data
	 */
	if ((len > (fs->sector_size - 5 * fs->ate_size)) || (len > UINT16_MAX) ||
	    ((len > 0) && (data == NULL))) {
		return -EINVAL;
	}

#ifdef CONFIG_ZMS_NO_DOUBLE_WRITE
	uint64_t wlk_addr;

	/* find latest entry with same id */
#ifdef CONFIG_ZMS_LOOKUP_CACHE
	wlk_addr = fs->lookup_cache[zms_lookup_cache_pos(id)];

	if (wlk_addr == ZMS_LOOKUP_CACHE_NO_ADDR) {
		if (len > 0) {
			goto no_cached_entry;
		} else {
			/* skip delete entry for non-existing entry */
			return 0;
		}
	}
#else
	wlk_addr = fs->ate_wra;
#endif /* CONFIG_ZMS_LOOKUP_CACHE */
	uint64_t rd_addr = wlk_addr;

	/* Search for a previous valid ATE with the same ID */
	struct zms_ate wlk_ate;
	int prev_found = zms_find_ate_with_id(fs, id, wlk_addr, fs->ate_wra, &wlk_ate, &rd_addr);
	if (prev_found < 0) {
		return prev_found;
	}

	if (prev_found) {
		/* previous entry found */
		if (len > ZMS_DATA_IN_ATE_SIZE) {
			rd_addr &= ADDR_SECT_MASK;
			rd_addr += wlk_ate.offset;
		}

		if (len == 0) {
			/* do not try to compare with empty data */
			if (wlk_ate.len == 0U) {
				/* skip delete entry as it is already the
				 * last one
				 */
				return 0;
			}
		} else if (len == wlk_ate.len) {
			/* do not try to compare if lengths are not equal */
			/* compare the data and if equal return 0 */
			if (len <= ZMS_DATA_IN_ATE_SIZE) {
				rc = memcmp(&wlk_ate.data, data, len);
				if (!rc) {
					return 0;
				}
			} else {
				rc = zms_flash_block_cmp(fs, rd_addr, data, len);
				if (rc <= 0) {
					return rc;
				}
			}
		}
	} else {
		/* skip delete entry for non-existing entry */
		if (len == 0) {
			return 0;
		}
	}
#ifdef CONFIG_ZMS_LOOKUP_CACHE
no_cached_entry:
#endif /* CONFIG_ZMS_LOOKUP_CACHE */
#endif /* CONFIG_ZMS_NO_DOUBLE_WRITE */

	/* calculate required space if the entry contains data */
	if (data_size) {
		/* Leave space for delete ate */
		if (len > ZMS_DATA_IN_ATE_SIZE) {
			required_space = data_size + fs->ate_size;
		} else {
			required_space = fs->ate_size;
		}
	}

	k_mutex_lock(&fs->zms_lock, K_FOREVER);

	gc_count = 0;
	while (1) {
		if (gc_count == fs->sector_count) {
			/* gc'ed all sectors, no extra space will be created
			 * by extra gc.
			 */
			rc = -ENOSPC;
			goto end;
		}

		/* We need to make sure that we leave the ATE at address 0x0 of the sector
		 * empty (even for delete ATE). Otherwise, the fs->ate_wra will be decremented
		 * after this write by ate_size and it will underflow.
		 * So the first position of a sector (fs->ate_wra = 0x0) is forbidden for ATEs
		 * and the second position could be written only be a delete ATE.
		 */
		if ((SECTOR_OFFSET(fs->ate_wra)) &&
		    (fs->ate_wra >= (fs->data_wra + required_space)) &&
		    (SECTOR_OFFSET(fs->ate_wra - fs->ate_size) || !len)) {
			rc = zms_flash_write_entry(fs, id, data, len);
			if (rc) {
				goto end;
			}
			break;
		}
		rc = zms_sector_close(fs);
		if (rc) {
			LOG_ERR("Failed to close the sector, returned = %d", rc);
			goto end;
		}
		rc = zms_gc(fs);
		if (rc) {
			LOG_ERR("Garbage collection failed, returned = %d", rc);
			goto end;
		}
		gc_count++;
	}
	rc = len;
end:
	k_mutex_unlock(&fs->zms_lock);
	return rc;
}