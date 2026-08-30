int settings_fcb_dst(struct settings_fcb *cf)
{
	cf->cf_store.cs_itf = &settings_fcb_itf;
	settings_dst_register(&cf->cf_store);

	return 0;
}