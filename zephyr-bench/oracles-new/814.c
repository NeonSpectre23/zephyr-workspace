size_t strspn(const char *s,
	      const char *accept)
{
	const char *ins = s;

	while ((*s != '\0') && (strchr(accept, *s) != NULL)) {
		++s;
	}

	return s - ins;
}