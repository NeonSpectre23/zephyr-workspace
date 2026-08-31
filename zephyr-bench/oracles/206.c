int net_buf_user_data_copy(struct net_buf *dst, const struct net_buf *src)
{
	__ASSERT_NO_MSG(dst);
	__ASSERT_NO_MSG(src);

	if (dst == src) {
		return 0;
	}

	if (dst->user_data_size < src->user_data_size) {
		return -EINVAL;
	}

	memcpy(dst->user_data, src->user_data, src->user_data_size);

	return 0;
}