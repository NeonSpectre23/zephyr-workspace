static ALWAYS_INLINE
/** @endcond  */
psa_status_t psa_ps_remove(psa_storage_uid_t uid)
{
#ifdef CONFIG_SECURE_STORAGE_PS_IMPLEMENTATION_ITS
	return secure_storage_its_remove(ITS_CALLER_ID, uid);
#else
	return secure_storage_ps_remove(uid);
#endif
}