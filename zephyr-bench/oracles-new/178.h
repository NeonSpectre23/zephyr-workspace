static ALWAYS_INLINE
/** @endcond  */
psa_status_t psa_its_get_info(psa_storage_uid_t uid, struct psa_storage_info_t *p_info)
{
	return secure_storage_its_get_info(ITS_CALLER_ID, uid, p_info);
}