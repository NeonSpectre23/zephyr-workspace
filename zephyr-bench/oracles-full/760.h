static inline void net_if_flag_set(struct net_if *iface,
				   enum net_if_flag value)
{
	if (iface == NULL || iface->if_dev == NULL) {
		return;
	}

	atomic_set_bit(iface->if_dev->flags, value);
}