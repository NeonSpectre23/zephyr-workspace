static inline uint8_t sys_hashmap_load_factor(const struct sys_hashmap *map)
{
	if (map->data->n_buckets == 0) {
		return 0;
	}

	return (map->data->size * 100) / map->data->n_buckets;
}