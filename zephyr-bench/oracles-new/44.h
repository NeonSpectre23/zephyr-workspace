static inline int input_report_key(const struct device *dev,
				   uint16_t code, int32_t value, bool sync,
				   k_timeout_t timeout)
{
	return input_report(dev, INPUT_EV_KEY, code, !!value, sync, timeout);
}