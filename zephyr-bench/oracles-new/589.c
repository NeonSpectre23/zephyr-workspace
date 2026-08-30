int os_mgmt_client_reset(struct os_mgmt_client *client)
{
	struct net_buf *nb;
	int rc;

	k_mutex_lock(&mcummgr_os_client_grp_mutex, K_FOREVER);
	active_client = client;
	/* allocate buffer */
	nb = smp_client_buf_allocation(active_client->smp_client, MGMT_GROUP_ID_OS,
				       OS_MGMT_ID_RESET, MGMT_OP_WRITE, SMP_MCUMGR_VERSION_1);
	if (!nb) {
		active_client->status = MGMT_ERR_ENOMEM;
		goto end;
	}
	k_sem_reset(&mcummgr_os_client_grp_sem);
	rc = smp_client_send_cmd(active_client->smp_client, nb, reset_res_fn,
				 &mcummgr_os_client_grp_sem, CONFIG_SMP_CMD_DEFAULT_LIFE_TIME);
	if (rc) {
		active_client->status = rc;
		smp_packet_free(nb);
		goto end;
	}
	/* Wait for process end update event */
	k_sem_take(&mcummgr_os_client_grp_sem, K_FOREVER);
end:
	rc = active_client->status;
	active_client = NULL;
	k_mutex_unlock(&mcummgr_os_client_grp_mutex);
	return rc;
}