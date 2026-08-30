static inline size_t chunk_header_bytes(struct z_heap *h)
{
	return big_heap(h) ? 8 : 4;
}