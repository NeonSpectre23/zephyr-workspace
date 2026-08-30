int modem_ubx_run_script(struct modem_ubx *ubx, struct modem_ubx_script *script)
{
	int ret;
	bool wait_for_rsp = script->match.filter.class != 0;

	ret = k_sem_take(&ubx->script_running_sem, script->timeout);
	if (ret != 0) {
		return -EBUSY;
	}

	ubx->script = script;
	k_sem_reset(&ubx->script_stopped_sem);

	int tries = ubx->script->retry_count + 1;
	int32_t ms_per_attempt = (uint64_t)k_ticks_to_ms_floor64(script->timeout.ticks) / tries;

	do {
		ret = modem_pipe_transmit(ubx->pipe,
					  (const uint8_t *)ubx->script->request.buf,
					  ubx->script->request.len);

		if (wait_for_rsp) {
			ret = k_sem_take(&ubx->script_stopped_sem, K_MSEC(ms_per_attempt));
		}
		tries--;
	} while ((tries > 0) && (ret < 0));

	k_sem_give(&ubx->script_running_sem);

	return (ret > 0) ? 0 : ret;
}