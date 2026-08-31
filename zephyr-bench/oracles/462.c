void z_shell_history_purge(struct shell_history *history)
{
	while (remove_from_tail(history)) {
	}
	history->current = NULL;
}