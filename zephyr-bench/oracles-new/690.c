void rtio_executor_ok(struct rtio_iodev_sqe *iodev_sqe, int result)
{
	rtio_executor_done(iodev_sqe, result, true);
}