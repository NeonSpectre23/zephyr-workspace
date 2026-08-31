static inline void rtio_sqe_prep_delay(struct rtio_sqe *sqe,
				       k_timeout_t timeout,
				       void *userdata)
{
	memset(sqe, 0, sizeof(struct rtio_sqe));
	sqe->op = RTIO_OP_DELAY;
	sqe->prio = 0;
	sqe->iodev = NULL;
	sqe->delay.timeout = timeout;
	sqe->userdata = userdata;
}