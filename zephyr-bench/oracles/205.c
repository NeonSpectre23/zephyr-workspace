void net_buf_unref(struct net_buf *buf)
#endif
{
	__ASSERT_NO_MSG(buf);

	while (buf) {
		struct net_buf *frags = buf->frags;
		struct net_buf_pool *pool;

		__ASSERT(buf->ref, "buf %p double free", buf);
		if (!buf->ref) {
#if defined(CONFIG_NET_BUF_LOG)
			NET_BUF_ERR("%s():%d: buf %p double free", func, line,
				    buf);
#endif
			return;
		}
		NET_BUF_DBG("buf %p ref %u pool_id %u frags %p", buf, buf->ref,
			    buf->pool_id, buf->frags);

		if (--buf->ref > 0) {
			return;
		}

		buf->data = NULL;
		buf->frags = NULL;

		pool = net_buf_pool_get(buf->pool_id);

#if defined(CONFIG_NET_BUF_POOL_USAGE)
		atomic_inc(&pool->avail_count);
		__ASSERT_NO_MSG(atomic_get(&pool->avail_count) <= pool->buf_count);
#endif

		if (pool->destroy) {
			pool->destroy(buf);
		} else {
			net_buf_destroy(buf);
		}

		buf = frags;
	}
}