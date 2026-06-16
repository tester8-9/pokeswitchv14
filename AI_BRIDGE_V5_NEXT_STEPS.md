# AI Bridge V5: Force existing candidate entries

Your V4 log proved the candidate-score hook is firing. It repeatedly saw existing candidate rows with action IDs `2` and `3`, score `100`, and the enable-like byte at `0`.

V5 adds a controlled option to force existing candidate rows ON and assign them a huge score. This is different from forcing `p_PokeChangeEnable`: it modifies the candidate table that the higher-level state machine is actually building.

## First V5 test config

Use this section in `config.toml` after installing the V5 artifact:

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

force_existing_candidates = true
force_existing_action_min = 2
force_existing_action_max = 5
force_existing_score = 1000000
force_existing_enable = true
```

## Expected meanings

- Switches happen: breakthrough; action IDs 2-5 are switch candidates or usable action rows.
- No switches, no crash: this candidate layer is not the final action layer; upload the log.
- Crash/freeze: the field is live and the forced candidate shape is invalid; upload the log/timing.

