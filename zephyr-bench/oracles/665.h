static inline bool sys_hashmap_get(const struct sys_hashmap *map, uint64_t key, uint64_t *value)
{
	return map->api->get(map, key, value);
}