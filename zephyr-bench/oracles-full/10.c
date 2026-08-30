int cbprintf_package_convert(void *in_packaged,
			     size_t in_len,
			     cbprintf_convert_cb cb,
			     void *ctx,
			     uint32_t flags,
			     uint16_t *strl,
			     size_t strl_len)
{
	__ASSERT_NO_MSG(in_packaged != NULL);

	uint8_t *buf = in_packaged;
	uint32_t *buf32 = in_packaged;
	unsigned int args_size, ros_nbr, rws_nbr;
	bool fmt_present = flags & CBPRINTF_PACKAGE_CONVERT_PTR_CHECK ? true : false;
	bool rw_cpy;
	bool ro_cpy;
	struct cbprintf_package_desc *in_desc = in_packaged;

	in_len = in_len != 0 ? in_len : get_package_len(in_packaged);

	/* Get number of RO string indexes in the package and check if copying
	 * includes appending those strings.
	 */
	ros_nbr = in_desc->ro_str_cnt;
	ro_cpy = ros_nbr &&
		(flags & CBPRINTF_PACKAGE_CONVERT_RO_STR) == CBPRINTF_PACKAGE_CONVERT_RO_STR;

	/* Get number of RW string indexes in the package and check if copying
	 * includes appending those strings.
	 */
	rws_nbr = in_desc->rw_str_cnt;
	rw_cpy = rws_nbr > 0 &&
		 (flags & CBPRINTF_PACKAGE_CONVERT_RW_STR) == CBPRINTF_PACKAGE_CONVERT_RW_STR;

	/* If flags are not set or appending request without rw string indexes
	 * present is chosen, just do a simple copy (or length calculation).
	 * Assuming that it is the most common case.
	 */
	if (!rw_cpy && !ro_cpy) {
		if (cb) {
			cb(in_packaged, in_len, ctx);
		}

		return in_len;
	}

	/* If we got here, it means that coping will be more complex and will be
	 * done with strings appending.
	 * Retrieve the size of the arg list.
	 */
	args_size = in_desc->len * sizeof(int);

	int out_len;

	/* Pointer to array with string locations. Array starts with read-only
	 * string locations.
	 */
	const char *fmt = *(const char **)(buf + sizeof(void *));
	uint8_t *str_pos = &buf[args_size];
	size_t strl_cnt = 0;

	/* If null destination, just calculate output length. */
	if (cb == NULL) {
		out_len = (int)in_len;
		if (ro_cpy) {
			for (unsigned int i = 0; i < ros_nbr; i++) {
				const char *str = *(const char **)&buf32[*str_pos];
				int len = append_string(cb, NULL, str, 0);

				/* If possible store calculated string length. */
				if (strl && strl_cnt < strl_len) {
					strl[strl_cnt++] = (uint16_t)len;
				}
				out_len += len;
				str_pos++;
			}
		} else {
			str_pos += ros_nbr;
		}

		bool drop_ro_str_pos = !(flags &
					(CBPRINTF_PACKAGE_CONVERT_KEEP_RO_STR |
					 CBPRINTF_PACKAGE_CONVERT_RO_STR));

		/* Handle RW strings. */
		for (unsigned int i = 0; i < rws_nbr; i++) {
			uint8_t arg_idx = *str_pos++;
			uint8_t arg_pos = *str_pos++;
			const char *str = *(const char **)&buf32[arg_pos];
			bool is_ro = ptr_in_rodata(str);
			int len;

			if (IS_ENABLED(CONFIG_CBPRINTF_CONVERT_CHECK_PTR) &&
			    fmt_present && is_ptr(fmt, arg_idx)) {
				LOG_WRN("(unsigned) char * used for %%p argument. "
					"It's recommended to cast it to void * because "
					"it may cause misbehavior in certain "
					"configurations. String:\"%s\" argument:%d", fmt, arg_idx);
				/* Since location is being dropped, decrement
				 * output length by 2 (argument index + position)
				 */
				out_len -= 2;
				continue;
			}

			if (is_ro) {
				if (flags & CBPRINTF_PACKAGE_CONVERT_RO_STR) {
					goto calculate_string_length;
				} else {
					out_len -= drop_ro_str_pos ? 2 : 1;
				}
			} else if (flags & CBPRINTF_PACKAGE_CONVERT_RW_STR) {
calculate_string_length:
				len = append_string(cb, NULL, str, 0);

				/* If possible store calculated string length. */
				if (strl && strl_cnt < strl_len) {
					strl[strl_cnt++] = (uint16_t)len;
				}
				/* string length decremented by 1 because argument
				 * index is dropped.
				 */
				out_len += (len - 1);
			}
		}

		return out_len;
	}

	struct cbprintf_package_desc out_desc;
	/* At least one is copied in. */
	uint8_t cpy_str_pos[16];
	/* Up to one will be kept since if both types are kept it returns earlier. */
	uint8_t keep_str_pos[16];
	uint8_t scpy_cnt;
	uint8_t keep_cnt;
	uint8_t *dst;
	int rv;

	/* If read-only strings shall be appended to the output package copy
	 * their indexes to the local array, otherwise indicate that indexes
	 * shall remain in the output package.
	 */
	if (ro_cpy) {
		scpy_cnt = ros_nbr;
		keep_cnt = 0;
		dst = cpy_str_pos;
	} else if (ros_nbr && flags & CBPRINTF_PACKAGE_CONVERT_KEEP_RO_STR) {
		scpy_cnt = 0;
		keep_cnt = ros_nbr;
		dst = keep_str_pos;
	} else {
		scpy_cnt = 0;
		keep_cnt = 0;
		dst = NULL;
	}
	if (dst) {
		memcpy(dst, str_pos, ros_nbr);
	}
	str_pos += ros_nbr;

	/* Go through read-write strings and identify which shall be appended.
	 * Note that there may be read-only strings there. Use address evaluation
	 * to determine if strings is read-only.
	 */
	for (unsigned int i = 0; i < rws_nbr; i++) {
		uint8_t arg_idx = *str_pos++;
		uint8_t arg_pos = *str_pos++;
		const char *str = *(const char **)&buf32[arg_pos];
		bool is_ro = ptr_in_rodata(str);

		if (IS_ENABLED(CONFIG_CBPRINTF_CONVERT_CHECK_PTR) &&
		    fmt_present && is_ptr(fmt, arg_idx)) {
			continue;
		}

		if (is_ro) {
			if (flags & CBPRINTF_PACKAGE_CONVERT_RO_STR) {
				__ASSERT_NO_MSG(scpy_cnt < sizeof(cpy_str_pos));
				cpy_str_pos[scpy_cnt++] = arg_pos;
			} else if (flags & CBPRINTF_PACKAGE_CONVERT_KEEP_RO_STR) {
				__ASSERT_NO_MSG(keep_cnt < sizeof(keep_str_pos));
				keep_str_pos[keep_cnt++] = arg_pos;
			} else {
				/* Drop information about ro_str location. */
			}
		} else {
			if (flags & CBPRINTF_PACKAGE_CONVERT_RW_STR) {
				__ASSERT_NO_MSG(scpy_cnt < sizeof(cpy_str_pos));
				cpy_str_pos[scpy_cnt++] = arg_pos;
			} else {
				__ASSERT_NO_MSG(keep_cnt < sizeof(keep_str_pos));
				keep_str_pos[keep_cnt++] = arg_idx;
				keep_str_pos[keep_cnt++] = arg_pos;
			}
		}
	}

	/* Set amount of strings appended to the package. */
	out_desc.len = in_desc->len;
	out_desc.str_cnt = in_desc->str_cnt + scpy_cnt;
	out_desc.rw_str_cnt = (flags & CBPRINTF_PACKAGE_CONVERT_RW_STR) ? 0 : (keep_cnt / 2);
	out_desc.ro_str_cnt = (flags & CBPRINTF_PACKAGE_CONVERT_RO_STR) ? 0 :
			((flags & CBPRINTF_PACKAGE_CONVERT_KEEP_RO_STR) ? keep_cnt : 0);

	/* Temporary overwrite input descriptor to allow bulk transfer */
	struct cbprintf_package_desc in_desc_backup = *in_desc;
	*in_desc = out_desc;

	/* Copy package header and arguments. */
	rv = cb(in_packaged, args_size, ctx);
	if (rv < 0) {
		return rv;
	}
	out_len = rv;
	/* Restore input descriptor. */
	*in_desc = in_desc_backup;

	/* Copy string positions which are kept. */
	rv = cb(keep_str_pos, keep_cnt, ctx);
	if (rv < 0) {
		return rv;
	}
	out_len += rv;

	/* Copy appended strings from source package to destination. */
	size_t strs_len = in_len - (args_size + ros_nbr + 2 * rws_nbr);

	rv = cb(str_pos, strs_len, ctx);
	if (rv < 0) {
		return rv;
	}
	out_len += rv;

	/* Append strings */
	for (unsigned int i = 0; i < scpy_cnt; i++) {
		uint8_t loc = cpy_str_pos[i];
		const char *str = *(const char **)&buf32[loc];
		uint16_t str_len = (strl && (i < strl_len)) ? strl[i] : 0;

		rv = cb(&loc, 1, ctx);
		if (rv < 0) {
			return rv;
		}
		out_len += rv;

		rv = append_string(cb, ctx, str, str_len);
		if (rv < 0) {
			return rv;
		}
		out_len += rv;
	}

	/* Empty call (can be interpreted as flushing) */
	(void)cb(NULL, 0, ctx);

	return out_len;
}