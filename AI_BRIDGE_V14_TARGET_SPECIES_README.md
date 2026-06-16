# AI Bridge V14 - runtime species / target mapping survey

V13 found that the eval_target object contains the target Pokémon species as a u16 at offset +0x70.
Examples from the latest Ian log:

- eval_target ...6270: +0x70 = 0x032F = 815 Cinderace
- eval_target ...6A10: +0x70 = 0x035D = 861 Grimmsnarl
- eval_target ...C590: +0x70 = 0x01DB = 475 Gallade

V14 adds direct logging:

```text
[ai_bridge] move_eval ... target_species=815/Cinderace ...
[ai_bridge] species_probe eval_target ... species=861/Grimmsnarl
```

It also follows several pointers inside the candidate state and scans them for known current battle species. This is still survey-only; do not enable switch forcing for this test.

Use the same survey config as V13 unless asked otherwise.
