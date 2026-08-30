struct net_buf *net_buf_clone(struct net_buf *buf, k_timeout_t timeout)
{
	k_timepoint_t end = sys_timepoint_calc(timeout);
	struct net_buf_pool *pool;
	struct net_buf *clone;

	__ASSERT_NO_MSG(buf);

	pool = net_buf_pool_get(buf->pool_id);

	clone = net_buf_alloc_len(pool, 0, timeout);
	if (!clone) {
		return NULL;
	}

	/* If the pool supports data referencing use that. Otherwise
	 * we need to allocate new data and make a copy.
	 */
	if (pool->alloc->cb->ref && !(buf->flags & NET_BUF_EXTERNAL_DATA)) {
		clone->__buf = buf->__buf ? data_ref(buf, buf->__buf) : NULL;
		clone->data = buf->data;
		clone->len = buf->len;
		clone->size = buf->size;
	} else {
		size_t size = buf->size;

		timeout = sys_timepoint_timeout(end);

		clone->__buf = data_alloc(clone, &size, timeout);
		if (!clone->__buf || size < buf->size) {
			net_buf_destroy(clone);
			return NULL;
		}

		clone->size = size;
		clone->data = clone->__buf + net_buf_headroom(buf);
		net_buf_add_mem(clone, buf->data, buf->len);
	}

	/* user_data_size should be the same for buffers from the same pool */
	__ASSERT(buf->user_data_size == clone->user_data_size, "Unexpected user data size");

	memcpy(clone->user_data, buf->user_data, clone->user_data_size);

	return clone;
}