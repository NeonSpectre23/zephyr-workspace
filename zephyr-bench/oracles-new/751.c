void *sip_svc_get_controller(char *method)
{
	if (method == NULL) {
		LOG_ERR("controller is NULL");
		return NULL;
	}

	/**
	 * For more info on below code check @ref SIP_SVC_CONTROLLER_DEFINE()
	 */
	STRUCT_SECTION_FOREACH(sip_svc_controller, ctrl) {
		if (!strncmp(ctrl->method, method, SIP_SVC_SUBSYS_CONDUIT_NAME_LENGTH)) {
			return (void *)ctrl;
		}
	}

	LOG_ERR("controller couldn't be found");
	return NULL;
}