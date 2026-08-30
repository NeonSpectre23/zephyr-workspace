size_t strcspn(const char *s,
	       const char *reject)
{
	const char *ins = s;

	while ((*s != '\0') && (strchr(reject, *s) == NULL)) {
		++s;
	}

	return s - ins;
}