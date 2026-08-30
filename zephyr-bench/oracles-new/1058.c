void zcbor_map_decode_bulk_reset(struct zcbor_map_decode_key_val *map, size_t map_size)
{
	for (size_t map_index = 0; map_index < map_size; ++map_index) {
		map[map_index].found = false;
	}
}