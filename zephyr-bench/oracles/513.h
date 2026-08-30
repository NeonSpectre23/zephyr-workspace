static inline int32_t linear_interpolate(const int32_t *x_axis, const int32_t *y_axis, uint8_t len,
					 int32_t x)
{
	float rise, run, slope;
	int32_t x_shifted;
	uint8_t idx_low = 0;

	/* Handle out of bounds values */
	if (x <= x_axis[0]) {
		return y_axis[0];
	} else if (x >= x_axis[len - 1]) {
		return y_axis[len - 1];
	}

	/* Find the lower x axis bucket */
	while (x >= x_axis[idx_low + 1]) {
		idx_low++;
	}

	/* Shift input to origin */
	x_shifted = x - x_axis[idx_low];
	if (x_shifted == 0) {
		return y_axis[idx_low];
	}

	/* Local slope */
	rise = y_axis[idx_low + 1] - y_axis[idx_low];
	run = x_axis[idx_low + 1] - x_axis[idx_low];
	slope = rise / run;

	/* Apply slope, undo origin shift and round */
	return roundf(y_axis[idx_low] + (slope * x_shifted));
}