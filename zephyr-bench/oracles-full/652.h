static inline int input_report_abs(const struct device *dev,
				   uint16_t code, int32_t value, bool sync,
				   k_timeout_t timeout)
{
	return input_report(dev, INPUT_EV_ABS, code, value, sync, timeout);
}