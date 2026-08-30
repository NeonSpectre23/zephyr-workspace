int smp_reassembly_collect(struct smp_transport *smpt, const void *buf, uint16_t len)
{
	if (smpt->__reassembly.current == NULL) {
		/*
		 * Collecting the first fragment: need to allocate buffer for it and prepare
		 * the reassembly context.
		 */
		if (len >= sizeof(struct smp_hdr)) {
			uint16_t expected = sys_be16_to_cpu(((struct smp_hdr *)buf)->nh_len);

			/*
			 * The length field in the header does not count the header size,
			 * but the reassembly does so the size needs to be added to the number of
			 * expected bytes.
			 */
			expected += sizeof(struct smp_hdr);

			/* Joining net_bufs not supported yet */
			if (len > MCUMGR_TRANSPORT_NETBUF_SIZE ||
			    expected > MCUMGR_TRANSPORT_NETBUF_SIZE) {
				return -ENOSR;
			}

			if (len > expected) {
				return -EOVERFLOW;
			}

			smpt->__reassembly.current = smp_packet_alloc();
			if (smpt->__reassembly.current != NULL) {
				smpt->__reassembly.expected = expected;
			} else {
				return -ENOMEM;
			}
		} else {
			/* Not enough data to even collect header */
			return -ENODATA;
		}
	}

	/* len is expected to be > 0 */
	if (smpt->__reassembly.expected >= len) {
		net_buf_add_mem(smpt->__reassembly.current, buf, len);
		smpt->__reassembly.expected -= len;
	} else {
		/*
		 * A fragment is longer than the expected size and will not fit into the buffer.
		 */
		return -EOVERFLOW;
	}

	return smpt->__reassembly.expected;
}