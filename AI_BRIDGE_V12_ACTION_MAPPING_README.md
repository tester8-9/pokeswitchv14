# AI Bridge V12 — switch-action mapping

V12 adds a controlled one-shot probe to identify what the existing switch candidates mean.

Use it after score-survey mode has shown candidate rows like:

```text
action=2 enabled=0 score=100
action=3 enabled=0 score=100
```

The old force-existing mode enabled every matching candidate repeatedly. V12's `action_probe_*` settings force only one matching candidate, making it much easier to observe:

- which active Pokémon switches out,
- which replacement Pokémon is chosen,
- whether action=2 and action=3 correspond to active slots, bench slots, or action classes.

## Config: action 2 one-shot

Set:

```toml
action_probe_enabled = true
action_probe_action_id = 2
action_probe_score = 1000000
action_probe_max_uses = 1
force_existing_candidates = false
switch_policy_mode = 0
```

Run Ian for one switch event and note exactly which Pokémon switched. Then repeat with `action_probe_action_id = 3`.
