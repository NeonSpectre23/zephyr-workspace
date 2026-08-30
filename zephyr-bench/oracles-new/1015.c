static int write_read(const struct device *dev, const struct w1_slave_config *config,
		      const uint8_t *write_buf, size_t write_len,
		      uint8_t *read_buf, size_t read_len)
{
	int ret;

	ret = reset_select(dev, config);
	if (ret != 0) {
		return ret;
	}

	ret = w1_write_block(dev, write_buf, write_len);
	if (ret < 0) {
		return ret;
	}

	if (read_buf == NULL && read_len > 0) {
		return -EIO;
	}
	return w1_read_block(dev, read_buf, read_len);
}