static inline bool onoff_has_error(const struct onoff_manager *mgr)
{
	return (mgr->flags & ONOFF_FLAG_ERROR) != 0;
}