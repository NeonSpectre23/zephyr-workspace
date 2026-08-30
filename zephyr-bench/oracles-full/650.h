static inline void fs_dir_t_init(struct fs_dir_t *zdp)
{
	zdp->dirp = NULL;
	zdp->mp = NULL;
}