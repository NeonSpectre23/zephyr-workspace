const struct symtab_info *symtab_get(void)
{
	extern const struct symtab_info z_symtab;

	return &z_symtab;
}