static inline bool sys_hashmap_contains_key(const struct sys_hashmap *map, uint64_t key)
{
	return sys_hashmap_get(map, key, NULL);
}