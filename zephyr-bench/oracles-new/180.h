static ALWAYS_INLINE
/** @endcond  */
psa_status_t psa_its_set(psa_storage_uid_t uid, size_t data_length,
			 const void *p_data, psa_storage_create_flags_t create_flags)
{
	return secure_storage_its_set(ITS_CALLER_ID, uid, data_length, p_data, create_flags);
}