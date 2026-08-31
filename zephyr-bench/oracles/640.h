static inline uint32_t spsc_pbuf_capacity(struct spsc_pbuf *pb)
{
	return pb->common.len - sizeof(uint32_t);
}