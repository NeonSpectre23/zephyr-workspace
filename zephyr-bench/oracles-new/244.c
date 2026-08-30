int llext_load(struct llext_loader *ldr, const char *name, struct llext **ext,
	       const struct llext_load_param *ldr_parm)
{
	int ret;

	*ext = llext_by_name(name);

	k_mutex_lock(&llext_lock, K_FOREVER);

	if (*ext) {
		/* The use count is at least 1 */
		ret = (*ext)->use_count++;
		goto out;
	}

	*ext = llext_alloc_metadata(sizeof(struct llext));
	if (*ext == NULL) {
		LOG_ERR("Not enough memory for extension metadata");
		ret = -ENOMEM;
		goto out;
	}

	ret = do_llext_load(ldr, *ext, ldr_parm);
	if (ret < 0) {
		llext_free_metadata(*ext);
		*ext = NULL;
		goto out;
	}

	/* The (*ext)->name array is LLEXT_MAX_NAME_LEN + 1 bytes long */
	strncpy((*ext)->name, name, LLEXT_MAX_NAME_LEN);
	(*ext)->name[LLEXT_MAX_NAME_LEN] = '\0';
	(*ext)->use_count++;

	sys_slist_append(&llext_list, &(*ext)->llext_list);
	LOG_INF("Loaded extension %s", (*ext)->name);

out:
	k_mutex_unlock(&llext_lock);
	return ret;
}