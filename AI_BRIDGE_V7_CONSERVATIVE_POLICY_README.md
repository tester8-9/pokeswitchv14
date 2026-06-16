# AI Bridge V7 — conservative / VGC-style switch policy

V5 proved that normal trainer battles already create switch candidates (`action=2` and `action=3`).
V4/V5 logs showed those candidates appear as:

```text
action=2 enabled=0 score=100
action=3 enabled=0 score=100
```

So the native switch candidates are present, but they are disabled and low-score.  V5 forced them to `enabled=1 score=1000000`, which proved switching works but was far too aggressive.

V6 added a tunable score, but boosting every candidate to 500 was still too spammy.

V7 adds extra gates for more VGC-like behavior:

- only act when both switch candidates are present (`switch_require_candidate_count = 2`)
- enable at most one switch candidate per candidate-table hit (`switch_max_candidates_per_hit = 1`)
- cap total forced switches per battle (`switch_max_forces_per_battle`)
- cap per active slot/action (`switch_max_forces_per_action`)
- add a longer candidate-hook cooldown (`switch_cooldown_hits`)
- require native score to be exactly/near 100 (`switch_min_native_score`, `switch_max_native_score`)
- optionally alternate between action 2 and action 3

## Recommended first gameplay config

```toml
[ai_bridge]
active = true
install_candidate_hooks = true

output_readback_post_offset = 8211420
candidate_list_post_offset = 9342476
candidate_score_post_offset = 9342576
final_selection_post_offset = 9342816

hook_output_readback_post = false
hook_candidate_list_post = false
hook_candidate_score_post = true
hook_final_selection_post = false

log_hits = true
max_log_hits = 64
candidate_dump_count = 8

force_output_score = 0
force_pokechange_enable = -1
force_final_selection = false
force_final_action_id = 0
force_final_score = 1000000
force_candidate_entry = false
force_candidate_slot = 0
force_candidate_action_id = 0
force_candidate_score = 1000000
force_candidate_enable = true
force_existing_candidates = false
force_existing_action_min = 2
force_existing_action_max = 3
force_existing_score = 1000000
force_existing_enable = true

switch_policy_mode = 4
switch_min_hit = 1
switch_score = 125
switch_max_forces_per_battle = 2
switch_max_forces_per_action = 1
switch_cooldown_hits = 12
switch_action2_score = 0
switch_action3_score = 0
switch_max_candidates_per_hit = 1
switch_require_candidate_count = 2
switch_min_native_score = 100
switch_max_native_score = 100
switch_preferred_action = 0
switch_alternate_actions = true
switch_native_score_only = false
switch_disable_after_force = true
```

## If it still switches too much

Use native-score-only mode:

```toml
switch_policy_mode = 4
switch_native_score_only = true
switch_score = 100
switch_max_forces_per_battle = 1
switch_max_forces_per_action = 1
switch_cooldown_hits = 20
switch_require_candidate_count = 2
switch_max_candidates_per_hit = 1
```

This only enables the candidate and does not raise it above its native score.

## If it barely switches

Raise only a little:

```toml
switch_policy_mode = 4
switch_native_score_only = false
switch_score = 175
switch_max_forces_per_battle = 3
switch_max_forces_per_action = 1
switch_cooldown_hits = 10
switch_require_candidate_count = 2
switch_max_candidates_per_hit = 1
```

Avoid huge scores like 500 or 1000000 for gameplay. Those are proof/testing values, not VGC-like values.
