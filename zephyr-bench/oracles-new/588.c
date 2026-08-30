void os_mgmt_client_init(struct os_mgmt_client *client, struct smp_client_object *smp_client)
{
	client->smp_client = smp_client;
}