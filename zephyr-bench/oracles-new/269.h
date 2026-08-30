static inline void sys_sfnode_flags_set(sys_sfnode_t *node, uint8_t flags)
{
	__ASSERT((flags & ~SYS_SFLIST_FLAGS_MASK) == 0UL, "flags too large");
	node->next_and_flags = (uintptr_t)(z_sfnode_next_peek(node)) | flags;
}