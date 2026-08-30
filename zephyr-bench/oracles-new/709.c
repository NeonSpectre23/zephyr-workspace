int settings_fcb_src(struct settings_fcb *cf)
{
	int rc;

	cf->cf_fcb.f_version = SETTINGS_FCB_VERS;
	cf->cf_fcb.f_scratch_cnt = 1;

	while (1) {
		rc = fcb_init(settings_fcb_get_flash_area(), &cf->cf_fcb);
		if (rc) {
			return -EINVAL;
		}

		/*
		 * Check if system was reset in middle of emptying a sector.
		 * This situation is recognized by checking if the scratch block
		 * is missing.
		 */
		if (fcb_free_sector_cnt(&cf->cf_fcb) < 1) {

			rc = flash_area_flatten(cf->cf_fcb.fap,
					cf->cf_fcb.f_active.fe_sector->fs_off,
					cf->cf_fcb.f_active.fe_sector->fs_size);

			if (rc) {
				return -EIO;
			}
		} else {
			break;
		}
	}

	cf->cf_store.cs_itf = &settings_fcb_itf;
	settings_src_register(&cf->cf_store);

	return 0;
}