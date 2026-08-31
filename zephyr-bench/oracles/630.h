static inline void rtio_sqe_prep_await(struct rtio_sqe *sqe,
				       const struct rtio_iodev *iodev,
				       int8_t prio,
				       void *userdata)
{
	memset(sqe, 0, sizeof(struct rtio_sqe));
	sqe->op = RTIO_OP_AWAIT;
	sqe->prio = prio;
	sqe->iodev = iodev;
	sqe->userdata = userdata;
}