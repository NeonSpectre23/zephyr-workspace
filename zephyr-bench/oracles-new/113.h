static inline struct net_buf * __must_check net_buf_alloc(struct net_buf_pool *pool,
							  k_timeout_t timeout)
{
	return net_buf_alloc_fixed(pool, timeout);
}