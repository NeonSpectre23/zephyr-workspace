int jwt_add_payload(struct jwt_builder *builder,
		     int32_t exp,
		     int32_t iat,
		     const char *aud)
{
	struct jwt_payload payload = {
		.exp = exp,
		.iat = iat,
		.aud = aud,
	};

	base64_outch(builder, '.');
	int res = json_obj_encode(jwt_payload_desc,
				  ARRAY_SIZE(jwt_payload_desc),
				  &payload, base64_append_bytes, builder);

	base64_flush(builder);
	return res;
}