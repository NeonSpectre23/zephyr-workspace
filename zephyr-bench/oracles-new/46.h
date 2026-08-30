static inline int isgraph(int c)
{
	return ((' ' < c) && (c <= '~'));
}