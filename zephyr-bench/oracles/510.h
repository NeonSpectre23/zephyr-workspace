static inline uint32_t k_event_test(struct k_event *event, uint32_t events_mask)
{
	return k_event_wait(event, events_mask, false, K_NO_WAIT);
}