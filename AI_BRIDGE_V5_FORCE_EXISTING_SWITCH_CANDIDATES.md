# AI Bridge V5 — force existing switch candidates

The uploaded V4 log proved that candidate hooks are working and that the candidate state creates candidate entries with action IDs `2` and `3`, but those entries are disabled and score 100.

V5 adds a safer mutation mode:

```toml
force_all_existing_candidates = true
force_existing_candidates_on_score = true
force_existing_candidate_score = 1000000
```

This does not invent a new action ID. It only takes candidates the game already created and changes:

- enabled: `0 -> 1`
- score: `100 -> 1000000`

If those candidates are switch slots, this should cause a switch. If it does not, the next hook must be after the function that consumes `final_action`.
