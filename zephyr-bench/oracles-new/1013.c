static inline void write_data(const void *data, size_t len, STMESP_Type *const stm_esp)
{
	const uint8_t *p8 = data;
	const uint32_t *p32;
	uint32_t unaligned;

	if (!len) {
		return;
	}

	/* Start by writing using D8 or D16 until pointer is word aligned. */
	unaligned = (uintptr_t)data & 0x00000003UL;
	if (unaligned != 0) {
		unaligned = 4 - unaligned;
		unaligned = MIN(len, unaligned);

		len -= unaligned;

		switch (unaligned) {
		case 3:
			STM_D8(stm_esp, *p8++, false, false);
			STM_D16(stm_esp, *(uint16_t *)p8, false, false);
			p8 += sizeof(uint16_t);
			break;
		case 2:
			if (len) {
				STM_D16(stm_esp, *(uint16_t *)p8, false, false);
				p8 += sizeof(uint16_t);
			} else {
				/* If len 0 then it means that even though 2 bytes are
				 * to be copied we can have address which is not aligned
				 * to 2 bytes.
				 */
				STM_D8(stm_esp, *p8++, false, false);
				STM_D8(stm_esp, *p8++, false, false);
			}
			break;
		default:
			/* 1 byte to align. */
			STM_D8(stm_esp, *p8++, false, false);
		}
	}

	p32 = (const uint32_t *)p8;

	/* Use D32 to write as much data as possible. */
	while (len >= sizeof(uint32_t)) {
		STM_D32(stm_esp, *p32++, false, false);
		len -= sizeof(uint32_t);
	}

	/* Write tail using D16 or D8. Address is word aligned at that point. */
	if (len) {
		p8 = (const uint8_t *)p32;
		switch (len) {
		case 2:
			STM_D16(stm_esp, *(uint16_t *)p8, false, false);
			p8 += sizeof(uint16_t);
			break;
		case 3:
			STM_D16(stm_esp, *(uint16_t *)p8, false, false);
			p8 += sizeof(uint16_t);
			/* fallthrough */
		default:
			/* 1 byte to align. */
			STM_D8(stm_esp, *p8++, false, false);
			break;
		}
	}
}