static void dump(void *data, size_t len)
{
	uint8_t *buf = data;
	size_t i, width = 8;

	for (i = 0; i < len; i++) {

		if ((i % width) == 0) {
			printk("0x%08lx\t", POINTER_TO_INT(buf + i));
		}

		printk("%02x ", buf[i]);

		if (((i + 1) % width) == 0 || i == (len - 1)) {
			printk("\n");
		}
	}
}