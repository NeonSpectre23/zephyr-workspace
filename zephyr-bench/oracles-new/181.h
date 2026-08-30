static ALWAYS_INLINE
/** @endcond  */
psa_status_t psa_ps_get(psa_storage_uid_t uid, size_t data_offset,
			size_t data_size, void *p_data, size_t *p_data_length)
{
#ifdef CONFIG_SECURE_STORAGE_PS_IMPLEMENTATION_ITS
	return secure_storage_its_get(ITS_CALLER_ID, uid, data_offset,
				      data_size, p_data, p_data_length);
#else
	return secure_storage_ps_get(uid, data_offset, data_size, p_data, p_data_length);
#endif
}