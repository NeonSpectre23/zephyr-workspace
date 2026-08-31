void z_shell_history_put(struct shell_history *history, uint8_t *line,
			 size_t len)
{
	sys_dnode_t *node;
	struct shell_history_item *new, *h_prev_item;
	uint32_t total_len = len + offsetof(struct shell_history_item, data);

	z_shell_history_mode_exit(history);
	if (len == 0) {
		return;
	}

	node = sys_dlist_peek_head(&history->list);
	h_prev_item = CONTAINER_OF(node, struct shell_history_item, dnode);

	if (node &&
	   (h_prev_item->len == len) &&
	   (memcmp(h_prev_item->data, line, len) == 0)) {
		/* Same command as before, do not store */
		return;
	}

	for (;;) {
		new = k_heap_alloc(history->heap, total_len, K_NO_WAIT);
		if (new) {
			/* Got memory, add new item */
			break;
		} else if (!remove_from_tail(history)) {
			/* Nothing to remove, cannot allocate memory. */
			return;
		}
	}

	new->len = len;
	memcpy(new->data, line, len);
	sys_dlist_prepend(&history->list, &new->dnode);
}