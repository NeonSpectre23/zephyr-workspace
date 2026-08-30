void timespec_add(struct timespec *apb,
		  const struct timespec *a,
		  const struct timespec *b)
{
	apb->tv_nsec = a->tv_nsec + b->tv_nsec;
	apb->tv_sec = a->tv_sec + b->tv_sec;
	if (apb->tv_nsec >= NSEC_PER_SEC) {
		apb->tv_sec += 1;
		apb->tv_nsec -= NSEC_PER_SEC;
	}
}