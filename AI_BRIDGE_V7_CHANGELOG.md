# AI Bridge V7 changelog

- Added conservative mode `switch_policy_mode = 4`.
- Added max candidates per candidate-table hit.
- Added candidate-count requirement so partial/lone switch-candidate builds are ignored.
- Added native score min/max gate; default targets the observed native switch score of 100.
- Added preferred/alternating active-slot selection so both active Pokémon are not boosted every time.
- Added optional `switch_native_score_only` mode for ultra-conservative testing.
- Added optional `switch_disable_after_force` to keep later switch rows disabled after one row is boosted.
