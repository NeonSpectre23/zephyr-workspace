bool z_shell_history_get(struct shell_history *history, bool up,
			 uint8_t *dst, uint16_t *len)
{
	struct shell_history_item *h_item; /* history item */
	sys_dnode_t *l_item; /* list item */

	if (up) { /* button up */
		l_item = (history->current == NULL) ?
		sys_dlist_peek_head(&history->list) :
		sys_dlist_peek_next_no_check(&history->list, history->current);
	} else { /* button down */
		l_item = sys_dlist_peek_prev(&history->list, history->current);
	}

	history->current = l_item;
	if (l_item == NULL) {
		/* Reached the end of history. */
		*len = 0U;
		return false;
	}

	h_item = CONTAINER_OF(l_item, struct shell_history_item, dnode);
	memcpy(dst, h_item->data, h_item->len);
	*len = h_item->len;
	dst[*len] = '\0';
	return true;
}