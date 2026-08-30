static inline
int cbpprintf(cbprintf_cb out, void *ctx, void *packaged)
{
#if defined(CONFIG_CBPRINTF_PACKAGE_SUPPORT_TAGGED_ARGUMENTS)
	union cbprintf_package_hdr *hdr =
		(union cbprintf_package_hdr *)packaged;

	if ((hdr->desc.pkg_flags & CBPRINTF_PACKAGE_ARGS_ARE_TAGGED)
	    == CBPRINTF_PACKAGE_ARGS_ARE_TAGGED) {
		return cbpprintf_external(out, cbvprintf_tagged_args,
					  ctx, packaged);
	}
#endif

	return cbpprintf_external(out, cbvprintf, ctx, packaged);
}