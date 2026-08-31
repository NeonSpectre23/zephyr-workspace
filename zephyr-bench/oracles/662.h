static inline void sys_hashmap_clear(struct sys_hashmap *map, sys_hashmap_callback_t cb,
				     void *cookie)
{
	map->api->clear(map, cb, cookie);
}