# AI Bridge V8 Score Survey

V8 answers the question: can we find scores other than the switch candidate score?

Yes. V8 adds read-only raw memory logging around the candidate state and readback output. The current known switch candidates are action 2 and action 3 with native score 100 and enabled=0. To compare that with move scores, use the score survey config below and upload the new `yuzu_log.txt`.

Recommended score-survey config:

```toml
[ai_bridge]
active = true
install_candidate_hooks = true

output_readback_post_offset = 8211420
candidate_list_post_offset = 9342476
candidate_score_post_offset = 9342576
final_selection_post_offset = 9342816

hook_output_readback_post = true
hook_candidate_list_post = true
hook_candidate_score_post = true
hook_final_selection_post = false

log_hits = true
max_log_hits = 24
candidate_dump_count = 8

force_output_score = 0
force_pokechange_enable = -1
force_final_selection = false
force_candidate_entry = false
force_existing_candidates = false
switch_policy_mode = 0

score_survey_mode = true
score_survey_dump_state = true
score_survey_dump_readback_x0 = true
score_survey_dump_readback_out = true
score_survey_words = 48
```

Run Ian for one full turn with no force switching. Upload the log. We will compare:

- `candidate_score cand[...] action=2/3 enabled=0 score=100`
- `readback_post score=...`
- raw words near `candidate_state_raw` and `readback_x0_raw`

If move scores are stored near this state object, the raw dump will expose them. If not, the next v9 step is to hook the move-scoring function separately.
