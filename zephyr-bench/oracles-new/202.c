int jwt_init_builder(struct jwt_builder *builder,
		     char *buffer,
		     size_t buffer_size)
{
	builder->base = buffer;
	builder->buf = buffer;
	builder->len = buffer_size;
	builder->overflowed = false;
	builder->pending = 0;

	return jwt_add_header(builder);
}