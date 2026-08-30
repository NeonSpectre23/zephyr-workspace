static inline struct rtio_cqe *rtio_cqe_consume(struct rtio *r)
{
	SYS_PORT_TRACING_FUNC_ENTER(rtio, cqe_consume, r);
	struct mpsc_node *node;
	struct rtio_cqe *cqe = NULL;

#ifdef CONFIG_RTIO_CONSUME_SEM
	if (k_sem_take(r->consume_sem, K_NO_WAIT) != 0) {
		SYS_PORT_TRACING_FUNC_EXIT(rtio, cqe_consume, r, NULL);
		return NULL;
	}
#endif

	node = mpsc_pop(&r->cq);
	if (node == NULL) {
		SYS_PORT_TRACING_FUNC_EXIT(rtio, cqe_consume, r, NULL);
		return NULL;
	}
	cqe = CONTAINER_OF(node, struct rtio_cqe, q);

	SYS_PORT_TRACING_FUNC_EXIT(rtio, cqe_consume, r, cqe);
	return cqe;
}