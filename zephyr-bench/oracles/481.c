int zbus_obs_is_chan_notification_masked(const struct zbus_observer *obs,
					 const struct zbus_channel *chan, bool *masked)
{
	_ZBUS_ASSERT(obs != NULL, "obs is required");
	_ZBUS_ASSERT(chan != NULL, "chan is required");

	int err = -ESRCH;

	struct zbus_channel_observation *observation;
	struct zbus_channel_observation_mask *observation_mask;

	K_SPINLOCK(&obs_slock) {
		const int limit = chan->data->observers_end_idx;

		for (int16_t i = chan->data->observers_start_idx; i < limit; ++i) {
			STRUCT_SECTION_GET(zbus_channel_observation, i, &observation);
			STRUCT_SECTION_GET(zbus_channel_observation_mask, i, &observation_mask);

			__ASSERT(observation != NULL, "observation must be not NULL");

			if (observation->obs == obs) {
				*masked = observation_mask->enabled;

				err = 0;

				K_SPINLOCK_BREAK;
			}
		}
	}

	return err;
}