ssize_t json_escape(char *str, size_t *len, size_t buf_size)
{
	char *next; /* Points after next character to escape. */
	char *dest; /* Points after next place to write escaped character. */
	size_t escaped_len = json_calc_escaped_len(str, *len);

	if (escaped_len == *len) {
		/*
		 * If no escape is necessary, there is nothing to do.
		 */
		return 0;
	}

	if (escaped_len >= buf_size) {
		return -ENOMEM;
	}

	/*
	 * By walking backwards in the buffer from the end positions
	 * of both the original and escaped strings, we avoid using
	 * extra space. Characters in the original string are
	 * overwritten only after they have already been escaped.
	 */
	str[escaped_len] = '\0';
	for (next = &str[*len], dest = &str[escaped_len]; next != str;) {
		char next_c = *(--next);
		char escape = escape_as(next_c);

		if (escape) {
			*(--dest) = escape;
			*(--dest) = '\\';
		} else {
			*(--dest) = next_c;
		}
	}
	*len = escaped_len;

	return 0;
}