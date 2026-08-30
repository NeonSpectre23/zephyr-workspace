static inline void sys_put_le32(uint32_t val, uint8_t dst[4])
{
	sys_put_le16(val, dst);
	sys_put_le16(val >> 16, &dst[2]);
}