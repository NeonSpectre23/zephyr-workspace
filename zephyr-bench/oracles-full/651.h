static inline void fs_file_t_init(struct fs_file_t *zfp)
{
	zfp->filep = NULL;
	zfp->mp = NULL;
	zfp->flags = 0;
}