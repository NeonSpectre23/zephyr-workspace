int shared_multi_heap_add(struct shared_multi_heap_region *region, void *user_data)
{
	enum shared_multi_heap_attr attr;
	struct sys_heap *h;
	unsigned int slot;

	attr = region->attr;

	if (attr >= MAX_SHARED_MULTI_HEAP_ATTR) {
		return -EINVAL;
	}

	/* No more heaps available */
	if (smh_data[attr].heap_cnt >= MAX_MULTI_HEAPS) {
		return -ENOMEM;
	}

	slot = smh_data[attr].heap_cnt;
	h = &smh_data[attr].heap_pool[slot];

	sys_heap_init(h, (void *) region->addr, region->size);
	sys_multi_heap_add_heap(&shared_multi_heap, h, user_data);

	smh_data[attr].heap_cnt++;

	return 0;
}