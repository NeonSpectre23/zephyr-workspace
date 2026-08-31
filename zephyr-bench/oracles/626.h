static inline struct rtio_sqe *rtio_sqe_acquire(struct rtio *r)
{
	SYS_PORT_TRACING_FUNC_ENTER(rtio, sqe_acquire, r);
	struct rtio_iodev_sqe *iodev_sqe = rtio_sqe_pool_alloc(r->sqe_pool);

	if (iodev_sqe == NULL) {
		SYS_PORT_TRACING_FUNC_EXIT(rtio, sqe_acquire, r, NULL);
		return NULL;
	}

	mpsc_push(&r->sq, &iodev_sqe->q);

	SYS_PORT_TRACING_FUNC_EXIT(rtio, sqe_acquire, r, &iodev_sqe->sqe);
	return &iodev_sqe->sqe;
}