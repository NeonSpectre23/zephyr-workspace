uint32_t sys_winstream_read(struct sys_winstream *ws,
			    uint32_t *seq, char *buf, uint32_t buflen)
{
	uint32_t seq0 = *seq, start, end, wseq, len, behind, copy, suffix;

	do {
		start = ws->start; end = ws->end; wseq = ws->seq;
		READ_BARRIER();

		/* No change in buffer state or empty initial stream are easy */
		if (*seq == wseq || start == end) {
			*seq = wseq;
			return 0;
		}

		/* Underflow: we're trying to read from a spot farther
		 * back than start.  We dropped some bytes, so cheat
		 * and just drop them all to catch up.
		 */
		behind = wseq - *seq;
		if (behind > idx_sub(ws, ws->end, ws->start)) {
			*seq = wseq;
			return 0;
		}

		/* Copy data */
		copy = idx_sub(ws, ws->end, behind);
		len = min(buflen, behind);
		suffix = min(len, ws->len - copy);
		MEMCPY(buf, &ws->data[copy], suffix);
		if (len > suffix) {
			MEMCPY(buf + suffix, &ws->data[0], len - suffix);
		}
		*seq = seq0 + len;

		/* Check vs. the state we initially read and repeat if
		 * needed.  This can't loop forever even if the other
		 * side is stuck spamming writes: we'll run out of
		 * buffer space and exit via the underflow condition.
		 */
		READ_BARRIER();
	} while (start != ws->start || wseq != ws->seq);

	return len;
}