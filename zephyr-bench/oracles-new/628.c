void pm_device_children_action_run(const struct device *dev,
				   enum pm_device_action action,
				   pm_device_action_failed_cb_t failure_cb)
{
	struct pm_visitor_context visitor_context = {
		.failure_cb = failure_cb,
		.action = action
	};

	(void)device_supported_foreach(dev, pm_device_children_visitor, &visitor_context);
}