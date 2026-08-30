void img_mgmt_client_init(struct img_mgmt_client *client, struct smp_client_object *smp_client,
			  int image_list_size, struct mcumgr_image_data *image_list)
{
	client->smp_client = smp_client;
	client->image_list_length = image_list_size;
	client->image_list = image_list;
}