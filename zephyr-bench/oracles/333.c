void rtio_work_req_submit(struct rtio_work_req *req,
			  struct rtio_iodev_sqe *iodev_sqe,
			  rtio_work_submit_t handler)
{
	if (!req) {
		return;
	}

	if (!iodev_sqe || !handler) {
		k_mem_slab_free(&rtio_work_items_slab, req);
		return;
	}

	req->iodev_sqe = iodev_sqe;
	req->handler = handler;

	/** For now we're simply treating this as a FIFO queue. It may be
	 * desirable to expand this to handle queue ordering based on RTIO
	 * SQE priority.
	 */
	k_queue_append(&rtio_workq, req);
}