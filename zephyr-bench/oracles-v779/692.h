static inline void rtio_sqe_prep_callback(struct rtio_sqe *sqe,
					  rtio_callback_t callback,
					  void *arg0,
					  void *userdata)
{
	memset(sqe, 0, sizeof(struct rtio_sqe));
	sqe->op = RTIO_OP_CALLBACK;
	sqe->prio = 0;
	sqe->iodev = NULL;
	sqe->callback.callback = callback;
	sqe->callback.arg0 = arg0;
	sqe->userdata = userdata;
}