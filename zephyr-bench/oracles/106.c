bool log_cache_get(struct log_cache *cache, uintptr_t id, uint8_t **data)
{
	sys_snode_t *prev_node = NULL;
	struct log_cache_entry *entry;
	bool hit = false;

	LOG_CACHE_PRINT("cache_get for id %lx\n", id);
	SYS_SLIST_FOR_EACH_CONTAINER(&cache->active, entry, node) {
		LOG_CACHE_DBG_ENTRY("checking", entry);
		if (cache->cmp(entry->id, id)) {
			cache->hit++;
			hit = true;
			break;
		}

		if (&entry->node == sys_slist_peek_tail(&cache->active)) {
			break;
		}
		prev_node = &entry->node;
	}

	if (hit) {
		LOG_CACHE_DBG_ENTRY("moving up", entry);
		sys_slist_remove(&cache->active, prev_node, &entry->node);
		sys_slist_prepend(&cache->active, &entry->node);
	} else {
		cache->miss++;

		sys_snode_t *from_idle = sys_slist_get(&cache->idle);

		if (from_idle) {
			entry = CONTAINER_OF(from_idle, struct log_cache_entry, node);
		} else {
			LOG_CACHE_DBG_ENTRY("removing", entry);
			sys_slist_remove(&cache->active, prev_node, &entry->node);
		}
	}

	*data = entry->data;
	entry->id = id;

	return hit;
}