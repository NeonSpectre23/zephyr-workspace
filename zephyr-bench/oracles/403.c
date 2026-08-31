void sys_winstream_write(struct sys_winstream *ws,
			 const char *data, uint32_t len)
{
	uint32_t len0 = len, suffix;
	uint32_t start = ws->start, end = ws->end, seq = ws->seq;

	/* Overflow: if we're truncating then just reset the buffer.
	 * (Max bytes buffered is actually len-1 because start==end is
	 * reserved to mean "empty")
	 */
	if (len > ws->len - 1) {
		start = end;
		len = ws->len - 1;
	}

	/* Make room in the buffer by advancing start first (note same
	 * len-1 from above)
	 */
	len = min(len, ws->len);
	if (seq != 0) {
		uint32_t avail = (ws->len - 1) - idx_sub(ws, end, start);

		if (len > avail) {
			ws->start = idx_mod(ws, start + (len - avail));
			WRITE_BARRIER();
		}
	}

	/* Had to truncate? */
	if (len < len0) {
		ws->start = end;
		data += len0 - len;
	}

	suffix = min(len, ws->len - end);
	MEMCPY(&ws->data[end], data, suffix);
	if (len > suffix) {
		MEMCPY(&ws->data[0], data + suffix, len - suffix);
	}

	ws->end = idx_mod(ws, end + len);
	ws->seq += len0; /* seq represents dropped bytes too! */
	WRITE_BARRIER();
}