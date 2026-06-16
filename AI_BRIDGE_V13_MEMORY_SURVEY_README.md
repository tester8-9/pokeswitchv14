# AI Bridge V13 - Actor/target memory survey

V12 proved the deferred switch policy is correctly blocking switches when the move-score summary says a usable move exists. V13 does not change switching behavior by itself. It adds deeper logging for:

- candidate entry raw bytes
- evaluated target object
- move object
- actor/side object

This is for locating the runtime structures needed for true switch-in scoring: active slot, bench slot, species, HP, and side identity.

Use `switch_policy_mode = 0` for pure survey mode unless explicitly testing switching.
