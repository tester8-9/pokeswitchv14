# SWSH AI Bridge V6 - Tunable switch policy

V5 proved the key breakthrough: action IDs 2 and 3 are live switch/action candidates. Forcing existing candidates enabled and score=1000000 caused the trainer to switch.

V6 turns that proof-of-life into a configurable policy:

- `switch_policy_mode = 1`: enable switch candidates only, keep native score.
- `switch_policy_mode = 2`: enable switch candidates and raise score only if below `switch_score`.
- `switch_policy_mode = 3`: enable and set score every time, like V5, but with caps/cooldowns.

Recommended first playable configs:

## Conservative
```toml
[ai_bridge]
active = true
install_candidate_hooks = true
candidate_score_post_offset = 9342576
hook_candidate_score_post = true
log_hits = true
max_log_hits = 64
candidate_dump_count = 8
force_existing_candidates = false
force_existing_action_min = 2
force_existing_action_max = 3
switch_policy_mode = 2
switch_score = 250
switch_max_forces_per_battle = 4
switch_max_forces_per_action = 2
switch_cooldown_hits = 2
switch_min_hit = 1
switch_action2_score = 0
switch_action3_score = 0
```

## Hard / VGC-ish
```toml
[ai_bridge]
active = true
install_candidate_hooks = true
candidate_score_post_offset = 9342576
hook_candidate_score_post = true
log_hits = true
max_log_hits = 64
candidate_dump_count = 8
force_existing_candidates = false
force_existing_action_min = 2
force_existing_action_max = 3
switch_policy_mode = 2
switch_score = 500
switch_max_forces_per_battle = 6
switch_max_forces_per_action = 3
switch_cooldown_hits = 1
switch_min_hit = 1
switch_action2_score = 0
switch_action3_score = 0
```

## Proof / very aggressive
```toml
[ai_bridge]
active = true
install_candidate_hooks = true
candidate_score_post_offset = 9342576
hook_candidate_score_post = true
log_hits = true
max_log_hits = 64
candidate_dump_count = 8
force_existing_candidates = false
force_existing_action_min = 2
force_existing_action_max = 3
switch_policy_mode = 3
switch_score = 1000000
switch_max_forces_per_battle = 6
switch_max_forces_per_action = 3
switch_cooldown_hits = 0
switch_min_hit = 1
switch_action2_score = 0
switch_action3_score = 0
```

Notes:

- Leave `force_existing_candidates = false` when using V6 policy mode. That old V5 option is still present as a raw proof switch.
- We do not yet have a true Dynamax-state detector. The safest practical workaround is to cap switches per battle so the AI does not keep switching late into the fight when a gym leader's ace/Dynamax mon is expected to be out.
- V6 does not edit moves, species, teams, trainer class, music, dialogue, or trainer IDs.
