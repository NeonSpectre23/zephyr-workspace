int ztress_exec_count(uint32_t id)
{
	if (id >= context_cnt) {
		return -EINVAL;
	}

	return exec_cnt[id];
}