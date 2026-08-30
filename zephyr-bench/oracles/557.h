static inline size_t net_buf_get_max_used(struct net_buf_pool *pool)
{
	return (size_t)pool->max_used;
}