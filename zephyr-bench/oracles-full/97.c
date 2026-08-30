static int handle_msg(struct mqtt_sn_client *client, struct mqtt_sn_data rx_addr)
{
	int err;
	struct mqtt_sn_param p;

	err = mqtt_sn_decode_msg(&client->rx, &p);
	if (err) {
		return err;
	}

	LOG_INF("Got message of type %d", p.type);

	switch (p.type) {
	case MQTT_SN_MSG_TYPE_ADVERTISE:
		handle_advertise(client, &p.params.advertise, rx_addr);
		break;
	case MQTT_SN_MSG_TYPE_SEARCHGW:
		handle_searchgw(client, &p.params.searchgw);
		break;
	case MQTT_SN_MSG_TYPE_GWINFO:
		handle_gwinfo(client, &p.params.gwinfo, rx_addr);
		break;
	case MQTT_SN_MSG_TYPE_CONNACK:
		handle_connack(client, &p.params.connack);
		break;
	case MQTT_SN_MSG_TYPE_WILLTOPICREQ:
		handle_willtopicreq(client);
		break;
	case MQTT_SN_MSG_TYPE_WILLMSGREQ:
		handle_willmsgreq(client);
		break;
	case MQTT_SN_MSG_TYPE_REGISTER:
		handle_register(client, &p.params.reg);
		break;
	case MQTT_SN_MSG_TYPE_REGACK:
		handle_regack(client, &p.params.regack);
		break;
	case MQTT_SN_MSG_TYPE_PUBLISH:
		handle_publish(client, &p.params.publish);
		break;
	case MQTT_SN_MSG_TYPE_PUBACK:
		handle_puback(client, &p.params.puback);
		break;
	case MQTT_SN_MSG_TYPE_PUBREC:
		handle_pubrec(client, &p.params.pubrec);
		break;
	case MQTT_SN_MSG_TYPE_PUBREL:
		handle_pubrel(client, &p.params.pubrel);
		break;
	case MQTT_SN_MSG_TYPE_PUBCOMP:
		handle_pubcomp(client, &p.params.pubcomp);
		break;
	case MQTT_SN_MSG_TYPE_SUBACK:
		handle_suback(client, &p.params.suback);
		break;
	case MQTT_SN_MSG_TYPE_UNSUBACK:
		handle_unsuback(client, &p.params.unsuback);
		break;
	case MQTT_SN_MSG_TYPE_PINGREQ:
		handle_pingreq(client);
		break;
	case MQTT_SN_MSG_TYPE_PINGRESP:
		handle_pingresp(client);
		break;
	case MQTT_SN_MSG_TYPE_DISCONNECT:
		handle_disconnect(client, &p.params.disconnect);
		break;
	case MQTT_SN_MSG_TYPE_WILLTOPICRESP:
		handle_willtopicresp(client, &p.params.willtopicresp);
		break;
	case MQTT_SN_MSG_TYPE_WILLMSGRESP:
		handle_willmsgresp(client, &p.params.willmsgresp);
		break;
	default:
		LOG_ERR("Unexpected message type %d", p.type);
		break;
	}

	k_work_reschedule(&client->process_work, K_NO_WAIT);

	return 0;
}