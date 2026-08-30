void mgmt_callback_register(struct mgmt_callback *callback)
{
	sys_slist_append(&mgmt_callback_list, &callback->node);
}