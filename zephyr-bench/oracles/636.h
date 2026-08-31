static inline void rtio_sqe_prep_read_multishot(struct rtio_sqe *sqe,
						const struct rtio_iodev *iodev, int8_t prio,
						void *userdata)
{
	rtio_sqe_prep_read_with_pool(sqe, iodev, prio, userdata);
	sqe->flags |= RTIO_SQE_MULTISHOT;
}