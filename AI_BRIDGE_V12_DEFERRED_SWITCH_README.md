# AI Bridge V12 — Deferred move-score-gated switching

V11 showed an important timing problem: switch candidates are created before the full move-scoring window has been logged. V12 stores the live switch-candidate table when it appears, lets the move evaluator run, then applies the switch policy later from the readback hook once enough move scores exist.

Use `switch_policy_mode = 5` for the new deferred policy.

Recommended first V12 config:

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
max_log_hits = 128
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

switch_policy_mode = 5
switch_min_hit = 40
switch_score = 100
switch_max_forces_per_battle = 99
switch_max_forces_per_action = 1
switch_cooldown_hits = 0
switch_action2_score = 0
switch_action3_score = 0

switch_max_candidates_per_hit = 1
switch_require_candidate_count = 2
switch_min_native_score = 100
switch_max_native_score = 100
switch_preferred_action = 0
switch_alternate_actions = true
switch_native_score_only = true
switch_disable_after_force = true

switch_gate_use_move_summary = true
switch_gate_best_move_max = 0
switch_gate_require_negative_record = false
switch_gate_negative_record_threshold = -20
switch_gate_min_move_records = 12

score_survey_mode = true
score_survey_dump_state = false
score_survey_dump_readback_x0 = false
score_survey_dump_readback_out = false
score_survey_words = 32
```

Expected log markers:

- `deferred_capture`
- `deferred_switch attempting`
- `deferred_candidate_score switch_policy_v12`

If the best move score is positive, the deferred switch policy should block. If the best move score is 0 or worse, it may enable a native score-100 switch candidate.
