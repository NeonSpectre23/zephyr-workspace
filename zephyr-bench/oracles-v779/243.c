int onoff_sync_lock(struct onoff_sync_service *srv,
		    k_spinlock_key_t *keyp)
{
	*keyp = k_spin_lock(&srv->lock);
	return srv->count;
}