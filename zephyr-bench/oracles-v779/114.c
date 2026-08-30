void log_cache_put(struct log_cache *cache, uint8_t *data)
{
	struct log_cache_entry *entry = CONTAINER_OF(data, struct log_cache_entry, data[0]);

	LOG_CACHE_DBG_ENTRY("cache_put", entry);
	sys_slist_prepend(&cache->active, &entry->node);
}