# V6 changelog

- Added `switch_policy_mode` for practical switching instead of raw always-force.
- Added tunable `switch_score` so switching can be boosted without always overpowering every action.
- Added caps: `switch_max_forces_per_battle` and `switch_max_forces_per_action`.
- Added `switch_cooldown_hits` to reduce repeated switch spam.
- Added optional per-action scores for action 2 and action 3.
- Kept the original V5 `force_existing_candidates` proof mode, but it should be OFF for real play.
