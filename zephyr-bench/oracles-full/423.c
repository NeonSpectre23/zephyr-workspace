int settings_register_with_cprio(struct settings_handler *handler, int cprio)
{
	int rc = 0;

	STRUCT_SECTION_FOREACH(settings_handler_static, ch) {
		if (strcmp(handler->name, ch->name) == 0) {
			return -EEXIST;
		}
	}

	settings_lock_take();

	struct settings_handler *ch;
	SYS_SLIST_FOR_EACH_CONTAINER(&settings_handlers, ch, node) {
		if (strcmp(handler->name, ch->name) == 0) {
			rc = -EEXIST;
			goto end;
		}
	}

	handler->cprio = cprio;
	sys_slist_append(&settings_handlers, &handler->node);

end:
	settings_lock_release();
	return rc;
}