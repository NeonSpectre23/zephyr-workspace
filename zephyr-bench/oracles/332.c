struct rtio_work_req *rtio_work_req_alloc(void)
{
	struct rtio_work_req *req;
	int err;

	err = k_mem_slab_alloc(&rtio_work_items_slab, (void **)&req, K_NO_WAIT);
	if (err) {
		return NULL;
	}

	return req;
}