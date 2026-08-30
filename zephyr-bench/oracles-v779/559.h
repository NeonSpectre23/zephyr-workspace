static inline
int flash_params_get_erase_cap(const struct flash_parameters *p)
{
#if defined(CONFIG_FLASH_HAS_EXPLICIT_ERASE)
#if defined(CONFIG_FLASH_HAS_NO_EXPLICIT_ERASE)
	return (p->caps.no_explicit_erase) ? 0 : FLASH_ERASE_C_EXPLICIT;
#else
	ARG_UNUSED(p);
	return FLASH_ERASE_C_EXPLICIT;
#endif
#endif
	return 0;
}