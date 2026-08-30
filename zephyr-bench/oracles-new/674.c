void qsort_r(void *base, size_t nmemb, size_t size,
	     int (*comp3)(const void *a, const void *b, void *arg), void *arg)
{
	struct qsort_comp cmp = {
		.has3 = true,
		.arg = arg,
		{
			.comp3 = comp3
		}
	};

	heap_sort(base, nmemb, size, &cmp);
}