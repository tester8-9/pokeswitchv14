# AI Bridge V11 — Per-decision move-score summary reset

V10 successfully blocked switching when the current move summary had a positive best score (`best=6`).
However, the uploaded log also showed the same `best=6` summary being reused much later, which means the
move summary was stale across later decision windows.

V11 fixes that by:

- recording move score summaries even after `max_log_hits` stops printing full logs;
- marking a score window as consumed once the candidate gate checks it;
- clearing the move summary on the next move-evaluation readback;
- adding a `gen=` field to `move_eval` and `move_score_summary` log lines so we can see each new scoring generation.

Use the same v10 config. Look for:

```text
[ai_bridge] move_eval ... gen=...
[ai_bridge] move_score_summary ... gen=...
[ai_bridge] candidate_score move_summary_gate blocked/allowed ...
[ai_bridge] candidate_score switch_policy_v11 ...
```

If `gen` increases across later turns, the stale-summary bug is fixed.
