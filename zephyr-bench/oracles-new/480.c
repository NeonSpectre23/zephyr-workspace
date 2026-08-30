ssize_t nvs_write(struct nvs_fs *fs, uint16_t id, const void *data, size_t len)
{
	int rc, gc_count;
	struct nvs_gc_write_entry wrt_entry;
	size_t ate_size, data_size;
	struct nvs_ate wlk_ate;
	uint32_t wlk_addr, rd_addr;
	uint16_t required_space = 0U; /* no space, appropriate for delete ate */
	bool prev_found = false;

	if (!fs->ready) {
		LOG_ERR("NVS not initialized");
		return -EACCES;
	}

	ate_size = nvs_al_size(fs, sizeof(struct nvs_ate));
	data_size = nvs_al_size(fs, nvs_data_len_with_crc(len));

	/* The maximum data size is sector size - 4 ate
	 * where: 1 ate for data, 1 ate for sector close, 1 ate for gc done,
	 * and 1 ate to always allow a delete.
	 * Also take into account the data CRC that is appended at the end of the data field,
	 * if any.
	 */
	if ((data_size > (fs->sector_size - 4 * ate_size)) ||
	    ((len > 0) && (data == NULL))) {
		return -EINVAL;
	}

	/* find latest entry with same id */
#ifdef CONFIG_NVS_LOOKUP_CACHE
	wlk_addr = fs->lookup_cache[nvs_lookup_cache_pos(id)];

	if (wlk_addr == NVS_LOOKUP_CACHE_NO_ADDR) {
		goto no_cached_entry;
	}
#else
	wlk_addr = fs->ate_wra;
#endif
	rd_addr = wlk_addr;

	while (1) {
		rd_addr = wlk_addr;
		rc = nvs_prev_ate(fs, &wlk_addr, &wlk_ate);
		if (rc) {
			return rc;
		}
		if ((wlk_ate.id == id) &&
		    (nvs_ate_valid(fs, rd_addr, &wlk_ate))) {
			prev_found = true;
			break;
		}
		if (wlk_addr == fs->ate_wra) {
			break;
		}
	}

#ifdef CONFIG_NVS_LOOKUP_CACHE
no_cached_entry:
#endif

	if (prev_found) {
		/* previous entry found */
		rd_addr &= ADDR_SECT_MASK;
		rd_addr += wlk_ate.offset;

		if (len == 0) {
			/* do not try to compare with empty data */
			if (wlk_ate.len == 0U) {
				/* skip delete entry as it is already the
				 * last one
				 */
				return 0;
			}
		} else if (len + NVS_DATA_CRC_SIZE == wlk_ate.len) {
			/* do not try to compare if lengths are not equal */
			/* compare the data and if equal return 0 */
			/* note: data CRC is not taken into account here, as it has not yet been
			 * appended to the data buffer
			 */
			rc = nvs_flash_block_cmp(fs, rd_addr, data, len);
			if (rc <= 0) {
				return rc;
			}
		}
	} else {
		/* skip delete entry for non-existing entry */
		if (len == 0) {
			return 0;
		}
	}

	/* calculate required space if the entry contains data */
	if (data_size) {
		/* Leave space for delete ate */
		required_space = data_size + ate_size;
	}

	k_mutex_lock(&fs->nvs_lock, K_FOREVER);

	gc_count = 0;
	while (1) {
		if (gc_count == fs->sector_count) {
			/* gc'ed all sectors, no extra space will be created
			 * by extra gc.
			 */
			rc = -ENOSPC;
			goto end;
		}

		/* ATEs grow backwards within a sector. In delete-only scenarios,
		 * a sector may contain only delete ATEs and no data entries.
		 * Prevent ATE writes at current start of sector to avoid crossing
		 * into the previous sector.
		 */
		if (fs->ate_wra >= (fs->data_wra + required_space) &&
		    (fs->ate_wra & ADDR_OFFS_MASK) != 0) {

			rc = nvs_flash_wrt_entry(fs, id, data, len);
			if (rc) {
				goto end;
			}
			break;
		}

		rc = nvs_sector_close(fs);
		if (rc) {
			goto end;
		}

		/* Initialize pending write request for GC processing */
		if (gc_count == 0) {
			wrt_entry.id = id;
			wrt_entry.data = data;
			wrt_entry.len = len;
			wrt_entry.is_written = false;
		}

		rc = nvs_gc(fs, &wrt_entry);
		if (rc) {
			goto end;
		}
		gc_count++;

		/* Exit if the entry has been written during GC */
		if (wrt_entry.is_written) {
			break;
		}
	}
	rc = len;
end:
	k_mutex_unlock(&fs->nvs_lock);
	return rc;
}