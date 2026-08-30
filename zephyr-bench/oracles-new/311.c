void mgmt_callback_unregister(struct mgmt_callback *callback)
{
	(void)sys_slist_find_and_remove(&mgmt_callback_list, &callback->node);
}