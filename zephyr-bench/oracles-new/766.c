int smp_client_single_response(struct net_buf *nb, const struct smp_hdr *res_hdr)
{
	struct smp_client_cmd_req *cmd_req;
	smp_client_res_fn cb;
	void *user_data;

	/* Discover request for incoming response */
	cmd_req = smp_client_response_discover(res_hdr);
	LOG_DBG("Response Header len %d, flags %d OP: %d group %d id %d seq %d", res_hdr->nh_len,
		res_hdr->nh_flags, res_hdr->nh_op, res_hdr->nh_group, res_hdr->nh_id,
		res_hdr->nh_seq);

	if (cmd_req) {
		cb = cmd_req->cb;
		user_data = cmd_req->user_data;
		smp_client_cmd_req_free(cmd_req);
		if (cb) {
			cb(nb, user_data);
			return MGMT_ERR_EOK;
		}
	}

	return MGMT_ERR_ENOENT;
}