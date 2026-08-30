int settings_name_steq(const char *name, const char *key, const char **next)
{
	if (next) {
		*next = NULL;
	}

	if ((!name) || (!key)) {
		return 0;
	}

	/* name might come from flash directly, in flash the name would end
	 * with '=' or '\0' depending how storage is done. Flash reading is
	 * limited to what can be read
	 */

	while ((*key != '\0') && (*key == *name) &&
	       (*name != '\0') && (*name != SETTINGS_NAME_END)) {
		key++;
		name++;
	}

	if (*key != '\0') {
		return 0;
	}

	if (*name == SETTINGS_NAME_SEPARATOR) {
		if (next) {
			*next = name + 1;
		}
		return 1;
	}

	if ((*name == SETTINGS_NAME_END) || (*name == '\0')) {
		return 1;
	}

	return 0;
}