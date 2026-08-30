static ALWAYS_INLINE uint32_t sys_read32(mm_reg_t addr)
{
	uint32_t ret;

	__asm__ volatile("movl %1, %0"
			 : "=r"(ret)
			 : "m" (*(volatile uint32_t *)(uintptr_t) addr)
			 : "memory");

	return ret;
}