int jwt_sign(struct jwt_builder *builder,
	     const char *der_key,
	     size_t der_key_len)
{
	int ret;
	unsigned char sig[JWT_SIGNATURE_LEN];

	ret = jwt_sign_impl(builder, der_key, der_key_len, sig, sizeof(sig));
	if (ret < 0) {
		return ret;
	}

	base64_outch(builder, '.');
	base64_append_bytes(sig, sizeof(sig), builder);
	base64_flush(builder);

	return builder->overflowed ? -ENOMEM : 0;
}