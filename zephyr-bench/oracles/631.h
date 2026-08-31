static inline void rtio_sqe_prep_await_executor(struct rtio_sqe *sqe, int8_t prio, void *userdata)
{
	rtio_sqe_prep_await(sqe, NULL, prio, userdata);
}