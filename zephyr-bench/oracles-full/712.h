static inline void net_buf_destroy(struct net_buf *buf)
{
	struct net_buf_pool *pool = net_buf_pool_get(buf->pool_id);

	if (buf->__buf) {
		if (!(buf->flags & NET_BUF_EXTERNAL_DATA)) {
			pool->alloc->cb->unref(buf, buf->__buf);
		}
		buf->__buf = NULL;
	}

	k_lifo_put(&pool->free, buf);
}