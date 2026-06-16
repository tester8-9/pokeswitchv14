# AI Bridge V4 — Candidate-selection probe

Your latest log proved that forcing the AMX output fields worked, but did not switch. That means the readback output is only scoring a candidate that already exists. It is not the final trainer action.

V4 hooks the next layer: the candidate-selection state machine around these Sword v1.3.2 offsets:

- `0x008E8E0C` candidate list after initial build
- `0x008E8E70` candidate after AI script score is added
- `0x008E8F60` final selected candidate after best-candidate selection

The candidate table layout being tested is:

- `state + 0xC5` candidate count
- `state + 0xC6` current candidate cursor
- `state + 0xC8 + i*8 + 0` candidate action ID
- `state + 0xC8 + i*8 + 1` candidate enabled flag
- `state + 0xC8 + i*8 + 4` candidate score
- `state + 0xF8` final selection valid flag
- `state + 0xFC` final selected score
- `state + 0x100` final selected action ID

## First V4 test config

Use read-only logging first:

```toml
[ai_bridge]
active = true
install_candidate_hooks = true
output_readback_post_offset = 8211420
candidate_list_post_offset = 9342476
candidate_score_post_offset = 9342576
final_selection_post_offset = 9342816
hook_output_readback_post = false
hook_candidate_list_post = true
hook_candidate_score_post = true
hook_final_selection_post = true
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
```

Fight Ian for 1–2 turns and upload the new `yuzu_log.txt`. The important lines are:

- `[ai_bridge] candidate_list ...`
- `[ai_bridge] candidate_score ...`
- `[ai_bridge] final_selection ...`
- `[ai_bridge] ... cand[i] action=... enabled=... score=...`

## Do not force yet

Do not set `force_final_selection=true` until we see what real action IDs appear. Once the real action IDs are mapped, the next test can deliberately force action IDs 4, 5, 6, etc. to check whether switches are represented as candidate IDs or whether we need one layer deeper.
