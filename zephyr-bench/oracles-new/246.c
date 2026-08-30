int llext_unload(struct llext **ext)
{
	__ASSERT(*ext, "Expected non-null extension");
	struct llext *tmp = *ext;

	/* Flush pending log messages, as the deferred formatting may be referencing
	 * strings/args in the extension we are about to unload
	 */
	log_flush();

	k_mutex_lock(&llext_lock, K_FOREVER);

	__ASSERT(tmp->use_count, "A valid LLEXT cannot have a zero use-count!");

	if (tmp->use_count-- != 1) {
		unsigned int ret = tmp->use_count;

		k_mutex_unlock(&llext_lock);
		return ret;
	}

	/* FIXME: protect the global list */
	sys_slist_find_and_remove(&llext_list, &tmp->llext_list);

	llext_dependency_remove_all(tmp);

	*ext = NULL;
	k_mutex_unlock(&llext_lock);

	if (tmp->sect_hdrs_on_heap) {
		llext_free_metadata(tmp->sect_hdrs);
	}

	llext_free_regions(tmp);
	llext_free_metadata(tmp->sym_tab.syms);
	llext_free_metadata(tmp->exp_tab.syms);
	llext_free_metadata(tmp);

	return 0;
}