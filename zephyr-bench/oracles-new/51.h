__pinned_func
static inline bool k_is_user_context(void)
{
#ifdef CONFIG_USERSPACE
	return arch_is_user_context();
#else
	return false;
#endif
}