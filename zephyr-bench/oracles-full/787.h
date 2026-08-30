static inline struct rtio_cqe *rtio_cqe_consume_block(struct rtio *r)
{
	struct mpsc_node *node;
	struct rtio_cqe *cqe;

#ifdef CONFIG_RTIO_CONSUME_SEM
	k_sem_take(r->consume_sem, K_FOREVER);
#endif
	node = mpsc_pop(&r->cq);
	while (node == NULL) {
		Z_SPIN_DELAY(1);
		node = mpsc_pop(&r->cq);
	}
	cqe = CONTAINER_OF(node, struct rtio_cqe, q);

	return cqe;
}