static inline void sys_dlist_range_prepend(sys_dlist_t *dest,
					   sys_dnode_t *start, sys_dnode_t *last)
{
	sys_dnode_t *const head = dest->head;
	sys_dnode_t *const prev = start->prev;
	sys_dnode_t *const next = last->next;

	/* Remove the range from its current list. */
	prev->next = next;
	next->prev = prev;

	/* Prepend the range to the destination list. */
	last->next = head;
	start->prev = dest;

	head->prev = last;
	dest->head = start;
}