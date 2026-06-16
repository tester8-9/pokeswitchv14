# AI Bridge V9 — move-score accumulator

V8 showed the current evaluated move ID in the readback output object at +0xB0. V9 decodes that field, records per-move/per-target score deltas, and logs a running move_score_summary.

Use the same read-only survey config as v8. Important settings:

```toml
[ai_bridge]
active = true
install_candidate_hooks = true
hook_output_readback_post = true
hook_candidate_list_post = true
hook_candidate_score_post = true
hook_final_selection_post = false
log_hits = true
max_log_hits = 48
candidate_dump_count = 8
force_output_score = 0
force_pokechange_enable = -1
force_final_selection = false
force_candidate_entry = false
force_existing_candidates = false
switch_policy_mode = 0
score_survey_mode = true
score_survey_dump_state = false
score_survey_dump_readback_x0 = false
score_survey_dump_readback_out = false
score_survey_words = 48
```

Look for log lines beginning with:

```text
[ai_bridge] move_eval
[ai_bridge] move_score_summary
```

This build should not force switches. Its job is to measure how the normal AI scores moves so the next switching policy can compare switch utility against native move utility.
