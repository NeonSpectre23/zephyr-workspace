int cbvprintf_package(void *packaged, size_t len, uint32_t flags,
		      const char *fmt, va_list ap)
{
/*
 * Internally, a byte is used to store location of a string argument within a
 * package. MSB bit is set if string is read-only so effectively 7 bits are
 * used for index, which should be enough.
 */
#define STR_POS_RO_FLAG BIT(7)
#define STR_POS_MASK BIT_MASK(7)

/* Buffer offset abstraction for better code clarity. */
#define BUF_OFFSET (buf - (uintptr_t)buf0)

	uint8_t *buf0 = packaged;  /* buffer start (may be NULL) */
	uintptr_t buf = (uintptr_t)buf0; /* current buffer position */
	unsigned int size;         /* current argument's size */
	unsigned int align;        /* current argument's required alignment */
	uint8_t str_ptr_pos[16];   /* string pointer positions */
	uint8_t str_ptr_arg[16];   /* string pointer argument index */
	unsigned int s_idx = 0;    /* index into str_ptr_pos[] */
	unsigned int s_rw_cnt = 0; /* number of rw strings */
	unsigned int s_ro_cnt = 0; /* number of ro strings */
	int arg_idx	      = -1; /* Argument index. Preincremented thus starting from -1.*/
	unsigned int i;
	const char *s;
	bool parsing = false;
	/* Flag indicates that rw strings are stored as array with positions,
	 * instead of appending them to the package.
	 */
	bool rws_pos_en = !!(flags & CBPRINTF_PACKAGE_ADD_RW_STR_POS);
	/* Get number of first read only strings present in the string.
	 * There is always at least 1 (fmt) but flags can indicate more, e.g
	 * fixed prefix appended to all strings.
	 */
	int fros_cnt = 1 + Z_CBPRINTF_PACKAGE_FIRST_RO_STR_CNT_GET(flags);
	bool is_str_arg = false;
	union cbprintf_package_hdr *pkg_hdr = packaged;

	/* Buffer must be aligned at least to size of a pointer. */
	if ((uintptr_t)packaged % sizeof(void *)) {
		return -EFAULT;
	}

#if defined(__xtensa__)
	/* Xtensa requires package to be 16 bytes aligned. */
	if ((uintptr_t)packaged % CBPRINTF_PACKAGE_ALIGNMENT) {
		return -EFAULT;
	}
#endif

	/*
	 * Make room to store the arg list size, the number of
	 * appended writable strings and the number of appended
	 * read-only strings. They both occupy 1 byte each.
	 * Skip a byte. Then a uint32_t to store flags used to
	 * create the package.
	 *
	 * Given the next value to store is the format string pointer
	 * which is guaranteed to be at least 4 bytes, we just reserve
	 * multiple of pointer size for the above to preserve alignment.
	 *
	 * Refer to union cbprintf_package_hdr for more details.
	 */
	buf += sizeof(*pkg_hdr);

	/*
	 * When buf0 is NULL we don't store anything.
	 * Instead we count the needed space to store the data.
	 * In this case, incoming len argument indicates the anticipated
	 * buffer "misalignment" offset.
	 */
	if (buf0 == NULL) {
		buf += len % CBPRINTF_PACKAGE_ALIGNMENT;
		/*
		 * The space to store the data is represented by both the
		 * buffer offset as well as the extra string data to be
		 * appended. When only figuring out the needed space, we
		 * don't append anything. Instead, we reuse the len variable
		 * to sum the size of that data.
		 *
		 * Also, we subtract any initial misalignment offset from
		 * the total as this won't be part of the buffer. To avoid
		 * going negative with an unsigned variable, we add an offset
		 * (CBPRINTF_PACKAGE_ALIGNMENT) that will be removed before
		 * returning.
		 */
		len = CBPRINTF_PACKAGE_ALIGNMENT - (len % CBPRINTF_PACKAGE_ALIGNMENT);
	}

	/*
	 * Otherwise we must ensure we can store at least
	 * the pointer to the format string itself.
	 */
	if ((buf0 != NULL) && (BUF_OFFSET + sizeof(char *)) > len) {
		return -ENOSPC;
	}

	/*
	 * Then process the format string itself.
	 * Here we branch directly into the code processing strings
	 * which is in the middle of the following while() loop. That's the
	 * reason for the post-decrement on fmt as it will be incremented
	 * prior to the next (actually first) round of that loop.
	 */
	s = fmt;
	--fmt;
	align = VA_STACK_ALIGN(char *);
	size = sizeof(char *);
	goto process_string;

	while (true) {

#if defined(CONFIG_CBPRINTF_PACKAGE_SUPPORT_TAGGED_ARGUMENTS)
		if ((flags & CBPRINTF_PACKAGE_ARGS_ARE_TAGGED)
		    == CBPRINTF_PACKAGE_ARGS_ARE_TAGGED) {
			int arg_tag = va_arg(ap, int);

			/*
			 * Here we copy the tag over to the package.
			 */
			align = VA_STACK_ALIGN(int);
			size = sizeof(int);

			/* align destination buffer location */
			buf = ROUND_UP(buf, align);

			/* make sure the data fits */
			if (buf0 != NULL && BUF_OFFSET + size > len) {
				return -ENOSPC;
			}

			if (buf0 != NULL) {
				*(int *)buf = arg_tag;
			}

			buf += sizeof(int);

			if (arg_tag == CBPRINTF_PACKAGE_ARG_TYPE_END) {
				/* End of arguments */
				break;
			}

			/*
			 * There are lots of __fallthrough here since
			 * quite a few of the data types have the same
			 * storage size.
			 */
			switch (arg_tag) {
			case CBPRINTF_PACKAGE_ARG_TYPE_CHAR:
				__fallthrough;
			case CBPRINTF_PACKAGE_ARG_TYPE_UNSIGNED_CHAR:
				__fallthrough;
			case CBPRINTF_PACKAGE_ARG_TYPE_SHORT:
				__fallthrough;
			case CBPRINTF_PACKAGE_ARG_TYPE_UNSIGNED_SHORT:
				__fallthrough;
			case CBPRINTF_PACKAGE_ARG_TYPE_INT:
				__fallthrough;
			case CBPRINTF_PACKAGE_ARG_TYPE_UNSIGNED_INT:
				align = VA_STACK_ALIGN(int);
				size = sizeof(int);
				break;

			case CBPRINTF_PACKAGE_ARG_TYPE_LONG:
				__fallthrough;
			case CBPRINTF_PACKAGE_ARG_TYPE_UNSIGNED_LONG:
				align = VA_STACK_ALIGN(long);
				size = sizeof(long);
				break;

			case CBPRINTF_PACKAGE_ARG_TYPE_LONG_LONG:
				__fallthrough;
			case CBPRINTF_PACKAGE_ARG_TYPE_UNSIGNED_LONG_LONG:
				align = VA_STACK_ALIGN(long long);
				size = sizeof(long long);
				break;

			case CBPRINTF_PACKAGE_ARG_TYPE_FLOAT:
				__fallthrough;
			case CBPRINTF_PACKAGE_ARG_TYPE_DOUBLE:
				__fallthrough;
			case CBPRINTF_PACKAGE_ARG_TYPE_LONG_DOUBLE: {
				/*
				 * Handle floats separately as they may be
				 * held in a different register set.
				 */
				union { double d; long double ld; } v;

				if (arg_tag == CBPRINTF_PACKAGE_ARG_TYPE_LONG_DOUBLE) {
					v.ld = va_arg(ap, long double);
					align = VA_STACK_ALIGN(long double);
					size = sizeof(long double);
				} else {
					v.d = va_arg(ap, double);
					align = VA_STACK_ALIGN(double);
					size = sizeof(double);
				}

				/* align destination buffer location */
				buf = ROUND_UP(buf, align);
				if (buf0 != NULL) {
					/* make sure it fits */
					if ((BUF_OFFSET + size) > len) {
						return -ENOSPC;
					}
					if (Z_CBPRINTF_VA_STACK_LL_DBL_MEMCPY) {
						memcpy((void *)buf, (uint8_t *)&v, size);
					} else if (fmt[-1] == 'L') {
						*(long double *)buf = v.ld;
					} else {
						*(double *)buf = v.d;
					}
				}
				buf += size;
				parsing = false;
				continue;
			}

			case CBPRINTF_PACKAGE_ARG_TYPE_PTR_CHAR:
				is_str_arg = true;

				__fallthrough;
			case CBPRINTF_PACKAGE_ARG_TYPE_PTR_VOID:
				align = VA_STACK_ALIGN(void *);
				size = sizeof(void *);
				break;

			default:
				return -EINVAL;
			}

		} else
#endif /* CONFIG_CBPRINTF_PACKAGE_SUPPORT_TAGGED_ARGUMENTS */
		{
			/* Scan the format string */
			if (*++fmt == '\0') {
				break;
			}

			if (!parsing) {
				if (*fmt == '%') {
					parsing = true;
					arg_idx++;
					align = VA_STACK_ALIGN(int);
					size = sizeof(int);
				}
				continue;
			}
			switch (*fmt) {
			case '%':
				parsing = false;
				arg_idx--;
				continue;

			case '#':
			case '-':
			case '+':
			case ' ':
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
			case '.':
			case 'h':
			case 'l':
			case 'L':
				continue;

			case '*':
				break;

			case 'j':
				align = VA_STACK_ALIGN(intmax_t);
				size = sizeof(intmax_t);
				continue;

			case 'z':
				align = VA_STACK_ALIGN(size_t);
				size = sizeof(size_t);
				continue;

			case 't':
				align = VA_STACK_ALIGN(ptrdiff_t);
				size = sizeof(ptrdiff_t);
				continue;

			case 'c':
			case 'd':
			case 'i':
			case 'o':
			case 'u':
			case 'x':
			case 'X':
				if (fmt[-1] == 'l') {
					if (fmt[-2] == 'l') {
						align = VA_STACK_ALIGN(long long);
						size = sizeof(long long);
					} else {
						align = VA_STACK_ALIGN(long);
						size = sizeof(long);
					}
				}
				parsing = false;
				break;

			case 's':
				is_str_arg = true;

				__fallthrough;
			case 'p':
			case 'n':
				align = VA_STACK_ALIGN(void *);
				size = sizeof(void *);
				parsing = false;
				break;

			case 'a':
			case 'A':
			case 'e':
			case 'E':
			case 'f':
			case 'F':
			case 'g':
			case 'G': {
				/*
				 * Handle floats separately as they may be
				 * held in a different register set.
				 */
				union { double d; long double ld; } v;

				if (fmt[-1] == 'L') {
					v.ld = va_arg(ap, long double);
					align = VA_STACK_ALIGN(long double);
					size = sizeof(long double);
				} else {
					v.d = va_arg(ap, double);
					align = VA_STACK_ALIGN(double);
					size = sizeof(double);
				}
				/* align destination buffer location */
				buf = ROUND_UP(buf, align);
				if (buf0 != NULL) {
					/* make sure it fits */
					if (BUF_OFFSET + size > len) {
						return -ENOSPC;
					}
					if (Z_CBPRINTF_VA_STACK_LL_DBL_MEMCPY) {
						memcpy((void *)buf, (uint8_t *)&v, size);
					} else if (fmt[-1] == 'L') {
						*(long double *)buf = v.ld;
					} else {
						*(double *)buf = v.d;
					}
				}
				buf += size;
				parsing = false;
				continue;
			}

			default:
				parsing = false;
				continue;
			}
		}

		/* align destination buffer location */
		buf = ROUND_UP(buf, align);

		/* make sure the data fits */
		if ((buf0 != NULL) && (BUF_OFFSET + size) > len) {
			return -ENOSPC;
		}

		/* copy va_list data over to our buffer */
		if (is_str_arg) {
			s = va_arg(ap, char *);
process_string:
			if (buf0 != NULL) {
				*(const char **)buf = s;
			}

			bool is_ro = (fros_cnt-- > 0) ? true : ptr_in_rodata(s);
			bool do_ro = !!(flags & CBPRINTF_PACKAGE_ADD_RO_STR_POS);

			if (is_ro && !do_ro) {
				/* nothing to do */
			} else {
				uint32_t s_ptr_idx = BUF_OFFSET / sizeof(int);

				/*
				 * In the do_ro case we must consider
				 * room for possible STR_POS_RO_FLAG.
				 * Otherwise the index range is 8 bits
				 * and any overflow is caught later.
				 */
				if (do_ro && s_ptr_idx > STR_POS_MASK) {
					__ASSERT(false, "String with too many arguments");
					return -EINVAL;
				}

				if (s_idx >= ARRAY_SIZE(str_ptr_pos)) {
					__ASSERT(false, "str_ptr_pos[] too small");
					return -EINVAL;
				}

				if (buf0 != NULL) {
					/*
					 * Remember string pointer location.
					 * We will append non-ro strings later.
					 */
					str_ptr_pos[s_idx] = s_ptr_idx;
					str_ptr_arg[s_idx] = arg_idx;
					if (is_ro) {
						/* flag read-only string. */
						str_ptr_pos[s_idx] |= STR_POS_RO_FLAG;
						s_ro_cnt++;
					} else {
						s_rw_cnt++;
					}
				} else if (is_ro) {
					/*
					 * Add only pointer position prefix
					 * when counting strings.
					 */
					len += 1;
				} else if (rws_pos_en) {
					/*
					 * Add only pointer position prefix and
					 * argument index when counting strings.
					 */
					len += 2;
				} else {
					/*
					 * Add the string length, the final '\0'
					 * and size of the pointer position prefix.
					 */
					len += strlen(s) + 1 + 1;
				}

				s_idx++;
			}
			buf += sizeof(char *);

			is_str_arg = false;
		} else if (size == sizeof(int)) {
			int v = va_arg(ap, int);

			if (buf0 != NULL) {
				*(int *)buf = v;
			}
			buf += sizeof(int);
		} else if (size == sizeof(long)) {
			long v = va_arg(ap, long);

			if (buf0 != NULL) {
				*(long *)buf = v;
			}
			buf += sizeof(long);
		} else if (size == sizeof(long long)) {
			long long v = va_arg(ap, long long);

			if (buf0 != NULL) {
				if (Z_CBPRINTF_VA_STACK_LL_DBL_MEMCPY) {
					memcpy((void *)buf, (uint8_t *)&v, sizeof(long long));
				} else {
					*(long long *)buf = v;
				}
			}
			buf += sizeof(long long);
		} else {
			__ASSERT(false, "unexpected size %u", size);
			return -EINVAL;
		}
	}

	/*
	 * We remember the size of the argument list as a multiple of
	 * sizeof(int) and limit it to a 8-bit field. That means 1020 bytes
	 * worth of va_list, or about 127 arguments on a 64-bit system
	 * (twice that on 32-bit systems). That ought to be good enough.
	 */
	if ((BUF_OFFSET / sizeof(int)) > 255) {
		__ASSERT(false, "too many format args");
		return -EINVAL;
	}

	/*
	 * If all we wanted was to count required buffer size
	 * then we have it now.
	 */
	if (buf0 == NULL) {
		return BUF_OFFSET + len - CBPRINTF_PACKAGE_ALIGNMENT;
	}

	/* Clear our buffer header. We made room for it initially. */
	*(char **)buf0 = NULL;

	/* Record end of argument list. */
	pkg_hdr->desc.len = BUF_OFFSET / sizeof(int);

	if (rws_pos_en) {
		/* Strings are appended, update location counter. */
		pkg_hdr->desc.str_cnt = 0;
		pkg_hdr->desc.rw_str_cnt = s_rw_cnt;
	} else {
		/* Strings are appended, update append counter. */
		pkg_hdr->desc.str_cnt = s_rw_cnt;
		pkg_hdr->desc.rw_str_cnt = 0;
	}

	pkg_hdr->desc.ro_str_cnt = s_ro_cnt;

#ifdef CONFIG_CBPRINTF_PACKAGE_HEADER_STORE_CREATION_FLAGS
	pkg_hdr->desc.pkg_flags = flags;
#endif

	/* Store strings pointer locations of read only strings. */
	if (s_ro_cnt != 0U) {
		for (i = 0; i < s_idx; i++) {
			if (!(str_ptr_pos[i] & STR_POS_RO_FLAG)) {
				continue;
			}

			uint8_t pos = str_ptr_pos[i] & STR_POS_MASK;

			/* make sure it fits */
			if ((BUF_OFFSET + 1) > len) {
				return -ENOSPC;
			}
			/* store the pointer position prefix */
			*(uint8_t *)buf = pos;
			++buf;
		}
	}

	/* Store strings prefixed by their pointer location. */
	for (i = 0; i < s_idx; i++) {
		/* Process only RW strings. */
		if (s_ro_cnt && str_ptr_pos[i] & STR_POS_RO_FLAG) {
			continue;
		}

		if (rws_pos_en) {
			size = 0;
			*(uint8_t *)buf = str_ptr_arg[i];
			++buf;
		} else {
			/* retrieve the string pointer */
			s = *(char **)(buf0 + str_ptr_pos[i] * sizeof(int));
			/* clear the in-buffer pointer (less entropy if compressed) */
			*(char **)(buf0 + str_ptr_pos[i] * sizeof(int)) = NULL;
			/* find the string length including terminating '\0' */
			size = strlen(s) + 1;
		}

		/* make sure it fits */
		if ((BUF_OFFSET + 1 + size) > len) {
			return -ENOSPC;
		}
		/* store the pointer position prefix */
		*(uint8_t *)buf = str_ptr_pos[i];
		++buf;
		/* copy the string with its terminating '\0' */
		memcpy((void *)buf, (uint8_t *)s, size);
		buf += size;
	}

	/*
	 * TODO: remove pointers for appended strings since they're useless.
	 * TODO: explore leveraging same mechanism to remove alignment padding
	 */

	return BUF_OFFSET;

#undef BUF_OFFSET
#undef STR_POS_RO_FLAG
#undef STR_POS_MASK
}