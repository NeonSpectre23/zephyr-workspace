static inline struct mpsc_node *mpsc_pop(struct mpsc *q)
{
	struct mpsc_node *head;
	struct mpsc_node *tail = q->tail;
	struct mpsc_node *next = (struct mpsc_node *)mpsc_ptr_get(tail->next);

	/* Skip over the stub/sentinel */
	if (tail == &q->stub) {
		if (next == NULL) {
			return NULL;
		}

		q->tail = next;
		tail = next;
		next = (struct mpsc_node *)mpsc_ptr_get(next->next);
	}

	/* If next is non-NULL then a valid node is found, return it */
	if (next != NULL) {
		q->tail = next;
		return tail;
	}

	head = (struct mpsc_node *)mpsc_ptr_get(q->head);

	/* If next is NULL, and the tail != HEAD then the queue has pending
	 * updates that can't yet be accessed.
	 */
	if (tail != head) {
		return NULL;
	}

	mpsc_push(q, &q->stub);

	next = (struct mpsc_node *)mpsc_ptr_get(tail->next);

	if (next != NULL) {
		q->tail = next;
		return tail;
	}

	return NULL;
}