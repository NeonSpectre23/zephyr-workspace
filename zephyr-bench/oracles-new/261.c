log_format_func_t log_format_func_t_get(uint32_t log_type)
{
	return format_table[log_type];
}