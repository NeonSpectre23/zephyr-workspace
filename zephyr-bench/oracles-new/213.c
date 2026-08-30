int k_mem_domain_init(struct k_mem_domain *domain, uint8_t num_parts,
		      struct k_mem_partition *parts[])
{
	k_spinlock_key_t key;
	int ret = 0;

	CHECKIF(domain == NULL) {
		ret = -EINVAL;
		goto out;
	}

	CHECKIF(!(num_parts == 0U || parts != NULL)) {
		LOG_ERR("parts array is NULL and num_parts is nonzero");
		ret = -EINVAL;
		goto out;
	}

	CHECKIF(!(num_parts <= max_partitions)) {
		LOG_ERR("num_parts of %d exceeds maximum allowable partitions (%d)",
			num_parts, max_partitions);
		ret = -EINVAL;
		goto out;
	}

	key = k_spin_lock(&z_mem_domain_lock);

	domain->num_partitions = 0U;
	(void)memset(domain->partitions, 0, sizeof(domain->partitions));

#ifdef CONFIG_MEM_DOMAIN_HAS_THREAD_LIST
	sys_dlist_init(&domain->thread_mem_domain_list);
#endif /* CONFIG_MEM_DOMAIN_HAS_THREAD_LIST */

#ifdef CONFIG_ARCH_MEM_DOMAIN_DATA
	ret = arch_mem_domain_init(domain);

	if (ret != 0) {
		LOG_ERR("architecture-specific initialization failed for domain %p with %d",
			domain, ret);
		ret = -ENOMEM;
		goto unlock_out;
	}
#endif /* CONFIG_ARCH_MEM_DOMAIN_DATA */
	if (num_parts != 0U) {
		uint32_t i;

		for (i = 0U; i < num_parts; i++) {
			CHECKIF(!check_add_partition(domain, parts[i])) {
				LOG_ERR("invalid partition index %d (%p)",
					i, parts[i]);
				ret = -EINVAL;
				goto unlock_out;
			}

			domain->partitions[i] = *parts[i];
			domain->num_partitions++;
#ifdef CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API
			int ret2 = arch_mem_domain_partition_add(domain, i);

			ARG_UNUSED(ret2);
			CHECKIF(ret2 != 0) {
				ret = ret2;
			}
#endif /* CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API */
		}
	}

unlock_out:
	k_spin_unlock(&z_mem_domain_lock, key);

out:
	return ret;
}