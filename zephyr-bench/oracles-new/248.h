static inline bool sys_hashmap_remove(struct sys_hashmap *map, uint64_t key, uint64_t *value)
{
	return map->api->remove(map, key, value);
}