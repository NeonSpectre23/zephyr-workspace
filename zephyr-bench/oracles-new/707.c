int settings_delete(const char *name)
{
	return settings_save_one(name, NULL, 0);
}