static ALWAYS_INLINE void sys_write32(uint32_t data, mm_reg_t addr)
{
	__asm__ volatile("movl %0, %1"
			 :
			 : "r"(data), "m" (*(volatile uint32_t *)(uintptr_t) addr)
			 : "memory");
}