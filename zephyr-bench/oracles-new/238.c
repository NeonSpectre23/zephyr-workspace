const void *llext_find_sym(const struct llext_symtable *sym_table, const char *sym_name)
{
	if (sym_table == NULL) {
		/* Built-in symbol table */
#ifdef CONFIG_LLEXT_EXPORT_BUILTINS_BY_SLID
		/* 'sym_name' is actually a SLID to search for */
		uintptr_t slid = (uintptr_t)sym_name;

		/* TODO: perform a binary search instead of linear.
		 * Note that - as of writing - the llext_const_symbol_area
		 * section is sorted in ascending SLID order.
		 * (see scripts/build/llext_prepare_exptab.py)
		 */
		STRUCT_SECTION_FOREACH(llext_const_symbol, sym) {
			if (slid == sym->slid) {
				return sym->addr;
			}
		}
#else
		STRUCT_SECTION_FOREACH(llext_const_symbol, sym) {
			if (strcmp(sym->name, sym_name) == 0) {
				return sym->addr;
			}
		}
#endif
	} else {
		/* find symbols in module */
		for (size_t i = 0; i < sym_table->sym_cnt; i++) {
			if (strcmp(sym_table->syms[i].name, sym_name) == 0) {
				return sym_table->syms[i].addr;
			}
		}
	}

	return NULL;
}