static inline uint32_t sys_hash32(const void *str, size_t n)
{
	if (IS_ENABLED(CONFIG_SYS_HASH_FUNC32_CHOICE_IDENTITY)) {
		return sys_hash32_identity(str, n);
	}

	if (IS_ENABLED(CONFIG_SYS_HASH_FUNC32_CHOICE_DJB2)) {
		return sys_hash32_djb2(str, n);
	}

	if (IS_ENABLED(CONFIG_SYS_HASH_FUNC32_CHOICE_MURMUR3)) {
		return sys_hash32_murmur3(str, n);
	}

	__ASSERT(0, "No default 32-bit hash. See CONFIG_SYS_HASH_FUNC32_CHOICE");

	return 0;
}