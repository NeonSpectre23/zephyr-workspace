int cs_trace_defmt_init(cs_trace_defmt_cb cb)
{
	callback = cb;
	return 0;
}