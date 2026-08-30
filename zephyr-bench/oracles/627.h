static inline int rtio_sqe_acquire_array(struct rtio *r, size_t n, struct rtio_sqe **sqes)
{
	struct rtio_iodev_sqe *iodev_sqe;
	size_t i;

	for (i = 0; i < n; i++) {
		iodev_sqe = rtio_sqe_pool_alloc(r->sqe_pool);
		if (iodev_sqe == NULL) {
			break;
		}
		sqes[i] = &iodev_sqe->sqe;
	}

	/* Not enough SQEs in the pool */
	if (i < n) {
		while (i > 0) {
			i--;
			iodev_sqe = CONTAINER_OF(sqes[i], struct rtio_iodev_sqe, sqe);
			rtio_sqe_pool_free(r->sqe_pool, iodev_sqe);
			sqes[i] = NULL;
		}

		return -ENOMEM;
	}

	for (i = 0; i < n; i++) {
		iodev_sqe = CONTAINER_OF(sqes[i], struct rtio_iodev_sqe, sqe);
		mpsc_push(&r->sq, &iodev_sqe->q);
	}

	return 0;
}