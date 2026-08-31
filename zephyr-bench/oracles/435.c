bool usbh_class_is_matching(const struct usbh_class_filter *const filter_rules,
			    const struct usbh_class_filter *const filter_data)
{
	/* Make empty filter set match everything (use class_api->probe() only) */
	if (filter_rules == NULL) {
		return true;
	}

	/* Try to find a rule that matches completely */
	for (size_t i = 0; filter_rules[i].flags != 0; i++) {
		const struct usbh_class_filter *rule = &filter_rules[i];

		if (rule->flags & USBH_CLASS_MATCH_VID_PID &&
		    (filter_data->vid != rule->vid || filter_data->pid != rule->pid)) {
			continue;
		}

		if (rule->flags & USBH_CLASS_MATCH_CODE_TRIPLE &&
		    (filter_data->class != rule->class || filter_data->sub != rule->sub ||
		     filter_data->proto != rule->proto)) {
			continue;
		}

		/* All the selected filter_rules did match */
		return true;
	}

	/* At the end of the filter table and still no match */
	return false;
}