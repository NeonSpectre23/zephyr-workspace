static void packet_end(STMESP_Type *stm_esp)
{
	if (IS_ENABLED(CONFIG_LOG_FRONTEND_STMESP_MSG_END_TIMESTAMP)) {
		STM_D8(stm_esp, 0, true, true);
	} else {
		STM_FLAG(stm_esp);
	}
	atomic_set(&new_data, 1);
}