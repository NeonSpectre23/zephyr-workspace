ZTESTABLE_STATIC int nvs_flash_al_wrt_streams(struct nvs_fs *fs, uint32_t addr,
					      const struct nvs_flash_wrt_stream *strm)
{
	const struct flash_parameters *fp = fs->flash_parameters;
	size_t wbs = fp->write_block_size;
	uint8_t buf[NVS_BLOCK_SIZE];
	size_t stream_idx = 0U;
	size_t full_bytes = 0U;
	size_t buf_fill = 0U;
	size_t copy = 0U;
	off_t offset;
	int rc;

	/* Nothing to write */
	if ((strm->head.len + strm->data.len + strm->tail.len) == 0U) {
		return 0;
	}

	/* Convert NVS address to flash offset */
	offset = fs->offset;
	offset += fs->sector_size * (addr >> ADDR_SECT_SHIFT);
	offset += addr & ADDR_OFFS_MASK;

	/* Logical write stream: head -> data -> tail */
	struct nvs_flash_buf streams[] = {
		strm->head,
		strm->data,
		strm->tail,
	};

	while (stream_idx < ARRAY_SIZE(streams)) {
		if (streams[stream_idx].len == 0U) {
			stream_idx++;
			continue;
		}

		/* Direct write of aligned full blocks */
		if (buf_fill == 0) {
			/* number of full blocks = len & ~(wbs - 1) */
			full_bytes = streams[stream_idx].len & ~(wbs - 1);

			if (full_bytes > 0U) {
				rc = flash_write(fs->flash_device, offset,
						 streams[stream_idx].ptr,
						 full_bytes);
				if (rc) {
					return rc;
				}

				streams[stream_idx].ptr += full_bytes;
				streams[stream_idx].len -= full_bytes;
				offset += full_bytes;
				continue;
			}
		}

		/* Copy to buffer to assemble a full block */
		copy = MIN(wbs - buf_fill, streams[stream_idx].len);
		if (copy > 0U) {
			(void)memcpy(buf + buf_fill, streams[stream_idx].ptr, copy);

			streams[stream_idx].ptr += copy;
			streams[stream_idx].len -= copy;
			buf_fill += copy;
		}

		/* If buffer full, write to flash */
		if (buf_fill == wbs) {
			rc = flash_write(fs->flash_device, offset, buf, wbs);
			if (rc) {
				return rc;
			}

			offset += wbs;
			buf_fill = 0U;
		}
	}


	if (buf_fill > 0U) {
		(void)memset(buf + buf_fill, fp->erase_value, wbs - buf_fill);

		rc = flash_write(fs->flash_device, offset, buf, wbs);
		if (rc) {
			return rc;
		}
	}

	return 0;
}