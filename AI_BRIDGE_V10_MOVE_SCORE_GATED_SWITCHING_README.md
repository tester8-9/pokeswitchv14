# AI Bridge V10 - move-score-gated switching

V9 discovered real move scoring summaries. Example from the user's Ian/062 log:

- Fake Out (move 252): best target sum +5, bad target -20
- Close Combat (move 370): best target 0, bad target -20
- Wide Guard (move 469): best target 0, bad target -20
- Detect (move 197): best target 0, bad target -20
- switch candidates: action 2 / 3, disabled, native score 100

V10 adds a gate that prevents switch boosting when at least one move still has a decent native score.

The recommended first gameplay config is:

```toml
[ai_bridge]
active = true
install_candidate_hooks = true

output_readback_post_offset = 8211420
candidate_list_post_offset = 9342476
candidate_score_post_offset = 9342576
final_selection_post_offset = 9342816

hook_output_readback_post = true
hook_candidate_list_post = false
hook_candidate_score_post = true
hook_final_selection_post = false

log_hits = true
max_log_hits = 64
candidate_dump_count = 8

force_output_score = 0
force_pokechange_enable = -1
force_final_selection = false
force_candidate_entry = false
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

switch_gate_use_move_summary = true
switch_gate_best_move_max = 0
switch_gate_require_negative_record = true
switch_gate_negative_record_threshold = -20
switch_gate_min_move_records = 8

score_survey_mode = true
score_survey_dump_state = false
score_survey_dump_readback_x0 = false
score_survey_dump_readback_out = false
score_survey_words = 32
```

Meaning:

- The bridge will only enable switch candidates when the move-score summary says the best move score is not positive.
- If there is a clearly good move available, such as Fake Out scoring +5 in the user's Ian log, the switch gate should block switching.
- This is not yet full matchup evaluation, but it is much closer to VGC logic than v5/v6/v7.

Look for these log lines:

- `move_summary_gate blocked: best_move_too_good`
- `move_summary_gate allowed`
- `switch_policy_v10`

