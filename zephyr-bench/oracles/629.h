static inline void rtio_sqe_drop_all(struct rtio *r)
{
	struct rtio_iodev_sqe *iodev_sqe;
	struct mpsc_node *node = mpsc_pop(&r->sq);

	while (node != NULL) {
		iodev_sqe = CONTAINER_OF(node, struct rtio_iodev_sqe, q);
		rtio_sqe_pool_free(r->sqe_pool, iodev_sqe);
		node = mpsc_pop(&r->sq);
	}
}