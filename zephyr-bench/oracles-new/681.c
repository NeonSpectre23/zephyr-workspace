uint8_t read_cb(const struct bt_gatt_attr *attr, uint16_t handle,
		       void *user_data)
{
	struct read_data *data = user_data;
	struct bt_att_chan *chan = data->chan;
	struct bt_conn *conn = chan->chan.chan.conn;
	int ret;

	LOG_DBG("handle 0x%04x", handle);

	/*
	 * If any attribute is founded in handle range it means that error
	 * should be changed from pre-set: invalid handle error to no error.
	 */
	data->err = 0x00;

	/* Check attribute permissions */
	data->err = bt_gatt_check_perm(conn, attr, BT_GATT_PERM_READ_MASK);
	if (data->err) {
		return BT_GATT_ITER_STOP;
	}

	/* Check the attribute authorization logic */
	if (!bt_gatt_attr_read_authorize(conn, attr)) {
		data->err = BT_ATT_ERR_AUTHORIZATION;
		return BT_GATT_ITER_STOP;
	}

	/* Read attribute value and store in the buffer */
	ret = att_chan_read(chan, attr, data->buf, data->offset, NULL, NULL);
	if (ret < 0) {
		data->err = err_to_att(ret);
		return BT_GATT_ITER_STOP;
	}

	return BT_GATT_ITER_CONTINUE;
}

static