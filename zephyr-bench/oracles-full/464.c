int sys_bitarray_test_bit(sys_bitarray_t *bitarray, size_t bit, int *val)
{
	k_spinlock_key_t key;
	int ret;
	size_t idx, off;

	__ASSERT_NO_MSG(bitarray != NULL);
	__ASSERT_NO_MSG(bitarray->num_bits > 0);

	key = k_spin_lock(&bitarray->lock);

	CHECKIF(val == NULL) {
		ret = -EINVAL;
		goto out;
	}

	if (bit >= bitarray->num_bits) {
		ret = -EINVAL;
		goto out;
	}

	idx = bit / bundle_bitness(bitarray);
	off = bit % bundle_bitness(bitarray);

	if ((bitarray->bundles[idx] & BIT(off)) != 0) {
		*val = 1;
	} else {
		*val = 0;
	}

	ret = 0;

out:
	k_spin_unlock(&bitarray->lock, key);
	return ret;
}