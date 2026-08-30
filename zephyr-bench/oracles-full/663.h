static inline struct k_work_delayable *
k_work_delayable_from_work(struct k_work *work)
{
	return CONTAINER_OF(work, struct k_work_delayable, work);
}