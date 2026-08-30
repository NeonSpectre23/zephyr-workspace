int mctp_i2c_gpio_target_unregister(struct mctp_binding_i2c_gpio_target *b)
{
	return i2c_target_unregister(b->i2c, &b->i2c_target_cfg);
}