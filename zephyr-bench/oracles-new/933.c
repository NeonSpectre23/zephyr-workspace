static void tx_done(struct bt_gatt_ots_l2cap *l2cap_ctx,
		    struct bt_conn *conn)
{
	/* Not doing any writes yet */
	LOG_ERR("Unexpected call, context: %p, conn: %p", l2cap_ctx, (void *)conn);
}