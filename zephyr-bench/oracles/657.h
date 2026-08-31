static inline void sys_dlist_range_append(sys_dlist_t *dest,
					  sys_dnode_t *start, sys_dnode_t *last)
{
	sys_dnode_t *const tail = dest->tail;
	sys_dnode_t *const prev = start->prev;
	sys_dnode_t *const next = last->next;

	/* Remove the range from its current list. */
	prev->next = next;
	next->prev = prev;

	/* Append the range to the destination list. */
	last->next = dest;
	start->prev = tail;

	tail->next = start;
	dest->tail = last;
}