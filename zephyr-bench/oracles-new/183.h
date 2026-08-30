static ALWAYS_INLINE
/** @endcond  */
psa_status_t psa_ps_set(psa_storage_uid_t uid, size_t data_length,
			const void *p_data, psa_storage_create_flags_t create_flags)
{
#ifdef CONFIG_SECURE_STORAGE_PS_IMPLEMENTATION_ITS
	return secure_storage_its_set(ITS_CALLER_ID, uid, data_length, p_data, create_flags);
#else
	return secure_storage_ps_set(uid, data_length, p_data, create_flags);
#endif
}