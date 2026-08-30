static ALWAYS_INLINE
/** @endcond  */
psa_status_t psa_its_remove(psa_storage_uid_t uid)
{
	return secure_storage_its_remove(ITS_CALLER_ID, uid);
}