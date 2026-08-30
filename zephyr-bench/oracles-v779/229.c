void net_pkt_unref(struct net_pkt *pkt)
{
#endif /* NET_LOG_LEVEL >= LOG_LEVEL_DBG */
	atomic_val_t ref;

	if (!pkt) {
#if NET_LOG_LEVEL >= LOG_LEVEL_DBG
		NET_ERR("*** ERROR *** pkt %p (%s():%d)", pkt, caller, line);
#endif
		return;
	}

	do {
		ref = atomic_get(&pkt->atomic_ref);
		if (!ref) {
#if NET_LOG_LEVEL >= LOG_LEVEL_DBG
			const char *func_freed;
			int line_freed;

			if (net_pkt_alloc_find(pkt, &func_freed, &line_freed)) {
				NET_ERR("*** ERROR *** pkt %p is freed already "
					"by %s():%d (%s():%d)",
					pkt, func_freed, line_freed, caller,
					line);
			} else {
				NET_ERR("*** ERROR *** pkt %p is freed already "
					"(%s():%d)", pkt, caller, line);
			}
#endif
			return;
		}
	} while (!atomic_cas(&pkt->atomic_ref, ref, ref - 1));

#if NET_LOG_LEVEL >= LOG_LEVEL_DBG
#if CONFIG_NET_PKT_LOG_LEVEL >= LOG_LEVEL_DBG
	NET_DBG("%s [%d] pkt %p ref %ld frags %p (%s():%d)",
		slab2str(pkt->slab), k_mem_slab_num_free_get(pkt->slab),
		pkt, ref - 1, pkt->frags, caller, line);
#endif
	if (ref > 1) {
		goto done;
	}

	frag = pkt->frags;
	while (frag) {
#if CONFIG_NET_PKT_LOG_LEVEL >= LOG_LEVEL_DBG
		NET_DBG("%s (%s) [%d] frag %p ref %d frags %p (%s():%d)",
			pool2str(net_buf_pool_get(frag->pool_id)),
			get_name(net_buf_pool_get(frag->pool_id)),
			get_frees(net_buf_pool_get(frag->pool_id)), frag,
			frag->ref - 1U, frag->frags, caller, line);
#endif

		if (!frag->ref) {
			const char *func_freed;
			int line_freed;

			if (net_pkt_alloc_find(frag,
					       &func_freed, &line_freed)) {
				NET_ERR("*** ERROR *** frag %p is freed "
					"already by %s():%d (%s():%d)",
					frag, func_freed, line_freed,
					caller, line);
			} else {
				NET_ERR("*** ERROR *** frag %p is freed "
					"already (%s():%d)",
					frag, caller, line);
			}
		}

		net_pkt_alloc_del(frag, caller, line);

		frag = frag->frags;
	}

	net_pkt_alloc_del(pkt, caller, line);
done:
#endif /* NET_LOG_LEVEL >= LOG_LEVEL_DBG */

	if (ref > 1) {
		return;
	}

	if (pkt->frags) {
		net_pkt_frag_unref(pkt->frags);
	}

	if (IS_ENABLED(CONFIG_NET_DEBUG_NET_PKT_NON_FRAGILE_ACCESS)) {
		pkt->buffer = NULL;
		net_pkt_cursor_init(pkt);
	}

	k_mem_slab_free(pkt->slab, (void *)pkt);
}