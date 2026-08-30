static void notify(struct bt_aics *inst, enum bt_aics_notify notify, const struct bt_uuid *uuid,
		   const void *data, uint16_t len)
{
	int err;

	err = bt_gatt_notify_uuid(NULL, uuid, inst->srv.service_p->attrs, data, len);
	if (err == -ENOMEM) {
		notify_work_reschedule(inst, notify, K_USEC(BT_AUDIO_NOTIFY_RETRY_DELAY_US));
	} else if (err < 0 && err != -ENOTCONN) {
		LOG_ERR("Notify %s err %d", aics_notify_str(notify), err);
	} else {
		/* Notification sent successfully */
	}
}