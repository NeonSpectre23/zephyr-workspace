static inline void rtio_sqe_prep_callback_no_cqe(struct rtio_sqe *sqe,
						 rtio_callback_t callback,
						 void *arg0,
						 void *userdata)
{
	rtio_sqe_prep_callback(sqe, callback, arg0, userdata);
	sqe->flags |= RTIO_SQE_NO_RESPONSE;
}