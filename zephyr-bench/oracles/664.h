static inline void sys_hashmap_foreach(const struct sys_hashmap *map, sys_hashmap_callback_t cb,
				       void *cookie)
{
	struct sys_hashmap_iterator it = {0};

	for (map->api->iter(map, &it); sys_hashmap_iterator_has_next(&it);) {
		it.next(&it);
		cb(it.key, it.value, cookie);
	}
}