int8_t otPlatRadioGetReceiveSensitivity(otInstance *aInstance)
{
	ARG_UNUSED(aInstance);

	return CONFIG_OPENTHREAD_DEFAULT_RX_SENSITIVITY;
}