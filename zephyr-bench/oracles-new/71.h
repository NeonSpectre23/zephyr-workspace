static inline bool linker_is_in_rodata(const void *addr)
{
#if defined(CONFIG_LINKER_USE_PINNED_SECTION)
	extern char lnkr_pinned_rodata_start[];
	extern char lnkr_pinned_rodata_end[];

	if (((const char *)addr >= (const char *)lnkr_pinned_rodata_start) &&
	    ((const char *)addr < (const char *)lnkr_pinned_rodata_end)) {
		return true;
	}
#endif

#if defined(CONFIG_ARM) || defined(CONFIG_ARC) || defined(CONFIG_X86) || \
	defined(CONFIG_ARM64) || defined(CONFIG_RISCV) || defined(CONFIG_SPARC) || \
	defined(CONFIG_MIPS) || defined(CONFIG_XTENSA) || defined(CONFIG_RX) || \
	defined(CONFIG_OPENRISC)
	extern char __rodata_region_start[];
	extern char __rodata_region_end[];
#define RO_START __rodata_region_start
#define RO_END   __rodata_region_end
#else
#define RO_START 0
#define RO_END   0
#endif

	return (((const char *)addr >= (const char *)RO_START) &&
		((const char *)addr < (const char *)RO_END));

#undef RO_START
#undef RO_END
}