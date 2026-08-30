int smp_client_send_cmd(struct smp_client_object *smp_client, struct net_buf *nb,
			smp_client_res_fn cb, void *user_data, int timeout_in_sec)
{
	struct smp_hdr smp_header;
	struct smp_client_cmd_req *cmd_req;

	if (timeout_in_sec > 30) {
		LOG_ERR("Command timeout can't be over 30 seconds");
		return MGMT_ERR_EINVAL;
	}

	if (timeout_in_sec == 0) {
		timeout_in_sec = CONFIG_SMP_CMD_DEFAULT_LIFE_TIME;
	}

	smp_read_hdr(nb, &smp_header);
	if (nb->len < sizeof(smp_header)) {
		return MGMT_ERR_EINVAL;
	}
	/* Update Length */
	smp_header.nh_len = sys_cpu_to_be16(nb->len - sizeof(smp_header));
	smp_header.nh_group = sys_cpu_to_be16(smp_header.nh_group),
	memcpy(nb->data, &smp_header, sizeof(smp_header));

	cmd_req = smp_client_cmd_req_allocate();
	if (!cmd_req) {
		return MGMT_ERR_ENOMEM;
	}

	LOG_DBG("Command send Header flags %d OP: %d group %d id %d seq %d", smp_header.nh_flags,
		smp_header.nh_op, sys_be16_to_cpu(smp_header.nh_group), smp_header.nh_id,
		smp_header.nh_seq);
	cmd_req->nb = nb;
	cmd_req->cb = cb;
	cmd_req->smp_client = smp_client;
	cmd_req->user_data = user_data;
	cmd_req->retry_cnt = timeout_in_sec * (1000 / CONFIG_SMP_CMD_RETRY_TIME);
	cmd_req->timestamp = k_uptime_get() + CONFIG_SMP_CMD_RETRY_TIME;
	/* Increment reference for re-transmission and read smp header */
	nb = net_buf_ref(nb);
	smp_cmd_add_to_list(cmd_req);
	k_fifo_put(&smp_client->tx_fifo, nb);
	k_work_submit_to_queue(smp_get_wq(), &smp_client->work);
	return MGMT_ERR_EOK;
}