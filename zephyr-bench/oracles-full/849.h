static inline uint8_t sys_sfnode_flags_get(const sys_sfnode_t *node)
{
	return node->next_and_flags & SYS_SFLIST_FLAGS_MASK;
}