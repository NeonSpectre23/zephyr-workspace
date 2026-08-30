static inline void mpsc_init(struct mpsc *q)
{
	mpsc_ptr_set(q->head, &q->stub);
	q->tail = &q->stub;
	mpsc_ptr_set(q->stub.next, NULL);
}