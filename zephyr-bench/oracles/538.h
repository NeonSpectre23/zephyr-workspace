static ALWAYS_INLINE void mpsc_push(struct mpsc *q, struct mpsc_node *n)
{
	struct mpsc_node *prev;
	int key;

	mpsc_ptr_set(n->next, NULL);

	key = arch_irq_lock();
	prev = (struct mpsc_node *)mpsc_ptr_set_get(q->head, n);
	mpsc_ptr_set(prev->next, n);
	arch_irq_unlock(key);
}