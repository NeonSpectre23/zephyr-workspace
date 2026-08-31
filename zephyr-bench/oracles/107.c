int log_cache_init(struct log_cache *cache, const struct log_cache_config *config)
{
	sys_slist_init(&cache->active);
	sys_slist_init(&cache->idle);

	size_t entry_size = ROUND_UP(sizeof(struct log_cache_entry) + config->item_size,
				     sizeof(uintptr_t));
	uint32_t entry_cnt = config->buf_len / entry_size;
	struct log_cache_entry *entry = config->buf;

	/* Ensure the cache has at least one entry */
	if (entry_cnt == 0) {
		return -EINVAL;
	}

	/* Add all entries to idle list */
	for (uint32_t i = 0; i < entry_cnt; i++) {
		sys_slist_append(&cache->idle, &entry->node);
		entry = (struct log_cache_entry *)((uintptr_t)entry + entry_size);
	}

	cache->cmp = config->cmp;
	cache->item_size = config->item_size;
	cache->hit = 0;
	cache->miss = 0;

	return 0;
}