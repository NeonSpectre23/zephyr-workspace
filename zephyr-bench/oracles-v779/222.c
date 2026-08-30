struct net_buf_pool *net_buf_pool_get(int id)
{
	struct net_buf_pool *pool;

	STRUCT_SECTION_GET(net_buf_pool, id, &pool);

	return pool;
}