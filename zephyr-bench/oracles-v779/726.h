static inline int sys_hashmap_insert(struct sys_hashmap *map, uint64_t key, uint64_t value,
				     uint64_t *old_value)
{
	return map->api->insert(map, key, value, old_value);
}