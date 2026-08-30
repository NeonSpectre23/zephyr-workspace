void smp_reassembly_init(struct smp_transport *smpt)
{
	smpt->__reassembly.current = NULL;
	smpt->__reassembly.expected = 0;
}