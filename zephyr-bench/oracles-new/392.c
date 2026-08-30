struct net_buf *net_buf_alloc_len(struct net_buf_pool *pool, size_t size,
				  k_timeout_t timeout)
#endif
{
	k_timepoint_t end = sys_timepoint_calc(timeout);
	struct net_buf *buf;
	k_spinlock_key_t key;

	__ASSERT_NO_MSG(pool);

	NET_BUF_DBG("%s():%d: pool %p size %zu", func, line, pool, size);

	/* We need to prevent race conditions
	 * when accessing pool->uninit_count.
	 */
	key = k_spin_lock(&pool->lock);

	/* If there are uninitialized buffers we're guaranteed to succeed
	 * with the allocation one way or another.
	 */
	if (pool->uninit_count) {
		uint16_t uninit_count;

		/* If this is not the first access to the pool, we can
		 * be opportunistic and try to fetch a previously used
		 * buffer from the LIFO with K_NO_WAIT.
		 */
		if (pool->uninit_count < pool->buf_count) {
			buf = k_lifo_get(&pool->free, K_NO_WAIT);
			if (buf) {
				k_spin_unlock(&pool->lock, key);
				goto success;
			}
		}

		uninit_count = pool->uninit_count--;
		k_spin_unlock(&pool->lock, key);

		buf = pool_get_uninit(pool, uninit_count);
		goto success;
	}

	k_spin_unlock(&pool->lock, key);

#if defined(CONFIG_NET_BUF_LOG) && (CONFIG_NET_BUF_LOG_LEVEL >= LOG_LEVEL_WRN)
	if (K_TIMEOUT_EQ(timeout, K_FOREVER)) {
		uint32_t ref = k_uptime_get_32();
		buf = k_lifo_get(&pool->free, K_NO_WAIT);
		while (!buf) {
#if defined(CONFIG_NET_BUF_POOL_USAGE)
			NET_BUF_WARN("%s():%d: Pool %s low on buffers.",
				     func, line, pool->name);
#else
			NET_BUF_WARN("%s():%d: Pool %p low on buffers.",
				     func, line, pool);
#endif
			buf = k_lifo_get(&pool->free, WARN_ALLOC_INTERVAL);
#if defined(CONFIG_NET_BUF_POOL_USAGE)
			NET_BUF_WARN("%s():%d: Pool %s blocked for %u secs",
				     func, line, pool->name,
				     (k_uptime_get_32() - ref) / MSEC_PER_SEC);
#else
			NET_BUF_WARN("%s():%d: Pool %p blocked for %u secs",
				     func, line, pool,
				     (k_uptime_get_32() - ref) / MSEC_PER_SEC);
#endif
		}
	} else {
		buf = k_lifo_get(&pool->free, timeout);
	}
#else
	buf = k_lifo_get(&pool->free, timeout);
#endif
	if (!buf) {
		NET_BUF_ERR("%s():%d: Failed to get free buffer", func, line);
		return NULL;
	}

success:
	NET_BUF_DBG("allocated buf %p", buf);

	if (size) {
		__maybe_unused size_t req_size = size;

		timeout = sys_timepoint_timeout(end);
		buf->__buf = data_alloc(buf, &size, timeout);
		if (!buf->__buf) {
			NET_BUF_ERR("%s():%d: Failed to allocate data",
				    func, line);
			net_buf_destroy(buf);
			return NULL;
		}

		__ASSERT_NO_MSG(req_size <= size);
	} else {
		buf->__buf = NULL;
	}

	buf->ref   = 1U;
	buf->flags = 0U;
	buf->frags = NULL;
	buf->size  = size;
	memset(buf->user_data, 0, buf->user_data_size);
	net_buf_reset(buf);

#if defined(CONFIG_NET_BUF_POOL_USAGE)
	atomic_dec(&pool->avail_count);
	__ASSERT_NO_MSG(atomic_get(&pool->avail_count) >= 0);
	pool->max_used = max(pool->max_used,
			     pool->buf_count - atomic_get(&pool->avail_count));
#endif
	return buf;
}