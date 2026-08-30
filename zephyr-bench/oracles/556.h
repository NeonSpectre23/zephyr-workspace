static inline size_t net_buf_get_available(struct net_buf_pool *pool)
{
	return (size_t)atomic_get(&pool->avail_count);
}