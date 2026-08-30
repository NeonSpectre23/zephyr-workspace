static inline void rtio_sqe_prep_nop(struct rtio_sqe *sqe,
				const struct rtio_iodev *iodev,
				void *userdata)
{
	memset(sqe, 0, sizeof(struct rtio_sqe));
	sqe->op = RTIO_OP_NOP;
	sqe->iodev = iodev;
	sqe->userdata = userdata;
}