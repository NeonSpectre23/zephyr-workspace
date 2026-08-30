int k_msgq_cleanup(struct k_msgq *msgq)
{
	void *mem;
	int ret = msgq_cleanup(msgq, &mem);

	k_free(mem);
	return ret;
}