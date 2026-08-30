bool zcbor_map_decode_bulk_key_found(struct zcbor_map_decode_key_val *map, size_t map_size,
	const char *key)
{
	size_t key_len;
	struct zcbor_map_decode_key_val *dptr = map;

	/* Lazy run, comparing pointers only assuming that compiler will be able to store
	 * read-only string of the same value as single instance.
	 */
	while (dptr < (map + map_size)) {
		if (dptr->key.value == (const uint8_t *)key) {
			return dptr->found;
		}
		++dptr;
	}

	/* Lazy run failed so need to do real comprison */
	key_len = strlen(key);
	dptr = map;

	while (dptr < (map + map_size)) {
		if (dptr->key.len == key_len &&
		    memcmp(key, dptr->key.value, key_len) == 0) {
			return dptr->found;
		}
		++dptr;
	}

	return false;
}