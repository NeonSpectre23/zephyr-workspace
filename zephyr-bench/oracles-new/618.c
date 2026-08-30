static void parse_ecclog(const struct device *dev, const uint64_t ecclog,
			 struct ibecc_error *error_data)
{
	struct ibecc_data *data = dev->data;

	if (ecclog == 0) {
		return;
	}

	error_data->type = ECC_ERROR_ERRTYPE(ecclog);
	error_data->address = ECC_ERROR_ERRADD(ecclog);
	error_data->syndrome = ECC_ERROR_ERRSYND(ecclog);

	if ((ecclog & ECC_ERROR_MERRSTS) != 0) {
		data->errors_uc++;
	}

	if ((ecclog & ECC_ERROR_CERRSTS) != 0) {
		data->errors_cor++;
	}
}