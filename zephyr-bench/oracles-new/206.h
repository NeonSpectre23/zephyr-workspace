static inline int rtio_sqe_copy_in(struct rtio *r, const struct rtio_sqe *sqes, size_t sqe_count)
{
	return rtio_sqe_copy_in_get_handles(r, sqes, NULL, sqe_count);
}