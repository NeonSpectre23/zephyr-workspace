static inline void rtio_sqe_prep_read_with_pool(struct rtio_sqe *sqe,
						const struct rtio_iodev *iodev, int8_t prio,
						void *userdata)
{
	rtio_sqe_prep_read(sqe, iodev, prio, NULL, 0, userdata);
	sqe->flags = RTIO_SQE_MEMPOOL_BUFFER;
}