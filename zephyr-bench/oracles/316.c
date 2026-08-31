void pm_policy_state_constraints_get(struct pm_state_constraints *constraints)
{
	for (int i = 0; i < constraints->count; i++) {
		pm_policy_state_lock_get(constraints->list[i].state,
					 constraints->list[i].substate_id);
	}
}