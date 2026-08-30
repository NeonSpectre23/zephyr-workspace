static inline void rtio_cqe_release(struct rtio *r, struct rtio_cqe *cqe)
{
	SYS_PORT_TRACING_FUNC(rtio, cqe_release, r, cqe);
	rtio_cqe_pool_free(r->cqe_pool, cqe);
}