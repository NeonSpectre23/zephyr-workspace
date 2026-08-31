void sys_multi_heap_add_heap(struct sys_multi_heap *mheap,
			struct sys_heap *heap, void *user_data)
{
	__ASSERT_NO_MSG(mheap->nheaps < ARRAY_SIZE(mheap->heaps));

	mheap->heaps[mheap->nheaps].heap = heap;
	mheap->heaps[mheap->nheaps++].user_data = user_data;

	/* Now sort them in memory order, simple extraction sort */
	for (int i = 0; i < mheap->nheaps; i++) {
		struct sys_multi_heap_rec swap;
		int lowest = -1;
		uintptr_t lowest_addr = UINTPTR_MAX;

		for (int j = i; j < mheap->nheaps; j++) {
			uintptr_t haddr = (uintptr_t)mheap->heaps[j].heap->heap;

			if (haddr < lowest_addr) {
				lowest = j;
				lowest_addr = haddr;
			}
		}
		swap = mheap->heaps[i];
		mheap->heaps[i] = mheap->heaps[lowest];
		mheap->heaps[lowest] = swap;
	}
}