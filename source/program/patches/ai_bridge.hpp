#pragma once

// SWSH AI Bridge V14 - runtime species / target mapping survey for switch-in scoring.
//
// What V3 proved from the user's log:
// - The hook at 0x7D4BDC is reached during Ian/062 trainer battle.
// - We can write the AMX output fields (score and PokeChangeEnable).
// - Forcing those fields does NOT cause switching.
//
// New conclusion:
// The AMX readback output is not the final trainer action.  It only scores
// the current candidate in a higher-level candidate-selection state machine.
// V4 hooks that state machine directly, logging candidate IDs/scores and
// optionally forcing the final selected candidate ID for controlled tests.

#include "lib.hpp"
#include "external.hpp"
#include "symbols.hpp"
#include "config.hpp"
#include "loggers.hpp"

namespace AIBridge {
    constexpr bool kOffsetsMappedForSword132 = true;

    constexpr u64 kOutputReadbackPostOffset = 0x007D4BDC;
    constexpr u64 kCandidateListPostOffset  = 0x008E8E0C;
    constexpr u64 kCandidateScorePostOffset = 0x008E8E70;
    constexpr u64 kFinalSelectionPostOffset = 0x008E8F60;

    inline bool installed = false;
    inline u32 output_hits = 0;
    inline u32 list_hits = 0;
    inline u32 score_hits = 0;
    inline u32 final_hits = 0;

    // V6 practical switching-policy state.  This is intentionally simple and
    // configurable.  It does not try to pretend we fully understand every
    // battle structure yet; it just prevents the v5 proof mode from becoming
    // "switch every time forever".
    inline u64 policy_state_ptr = 0;
    inline u32 policy_forced_total = 0;
    inline u32 policy_forced_action2 = 0;
    inline u32 policy_forced_action3 = 0;
    inline u32 policy_forced_other = 0;
    inline u32 policy_cooldown = 0;
    inline u32 policy_last_preferred_action = 3;

    // V12 deferred switching: candidate_score fires before the full move-score
    // window is known.  Store the live candidate table, then allow the
    // readback/move-scoring hook to apply the switch policy after enough move
    // evaluations have been accumulated.
    inline u64 deferred_candidate_state_ptr = 0;
    inline u32 deferred_candidate_score_hit = 0;
    inline bool deferred_candidate_ready = false;
    inline bool deferred_policy_applied_this_window = false;

    static inline bool ShouldLog(u32 hit) {
        return global_config.ai_bridge.log_hits && hit <= global_config.ai_bridge.max_log_hits;
    }

    static inline u32 ClampCandidateCount(u32 count) {
        if (count > 16) return 16;
        return count;
    }

    static inline void DumpCandidateTable(const char* tag, u64 state_ptr, u32 hit) {
        if (!ShouldLog(hit)) return;
        if (state_ptr < 0x100000) {
            Logging.Log("[ai_bridge] %s hit=%u invalid_state=%016lx\n", tag, hit, state_ptr);
            return;
        }

        volatile u8* b = reinterpret_cast<volatile u8*>(state_ptr);
        volatile u32* w = reinterpret_cast<volatile u32*>(state_ptr);

        const u32 count = ClampCandidateCount(static_cast<u32>(b[0xC5]));
        const u32 cursor = static_cast<u32>(b[0xC6]);
        const u32 final_valid = static_cast<u32>(b[0xF8]);
        const u32 final_score = *reinterpret_cast<volatile u32*>(state_ptr + 0xFC);
        const u32 final_action = *reinterpret_cast<volatile u32*>(state_ptr + 0x100);

        Logging.Log("[ai_bridge] %s hit=%u state=%016lx count=%u cursor=%u final_valid=%u final_score=%u final_action=%u c0=%u c4=%u\n",
            tag, hit, state_ptr, count, cursor, final_valid, final_score, final_action,
            *reinterpret_cast<volatile u32*>(state_ptr + 0xC0), static_cast<u32>(b[0xC4]));

        u32 dump_count = global_config.ai_bridge.candidate_dump_count;
        if (dump_count > count) dump_count = count;
        if (dump_count > 16) dump_count = 16;

        for (u32 i = 0; i < dump_count; i++) {
            const u64 entry = state_ptr + 0xC8 + (static_cast<u64>(i) * 8);
            const u32 action_id = static_cast<u32>(*reinterpret_cast<volatile u8*>(entry + 0x0));
            const u32 enabled = static_cast<u32>(*reinterpret_cast<volatile u8*>(entry + 0x1));
            const u32 byte2 = static_cast<u32>(*reinterpret_cast<volatile u8*>(entry + 0x2));
            const u32 byte3 = static_cast<u32>(*reinterpret_cast<volatile u8*>(entry + 0x3));
            const u32 score = *reinterpret_cast<volatile u32*>(entry + 0x4);
            Logging.Log("[ai_bridge] %s cand[%u] action=%u enabled=%u b2=%u b3=%u score=%u raw=%02x %02x %02x %02x %08x\n",
                tag, i, action_id, enabled, byte2, byte3, score,
                action_id, enabled, byte2, byte3, score);
        }
        (void)w;
    }


    static inline bool LooksLikePtr(u64 ptr) {
        return ptr >= 0x100000;
    }

    static inline u16 ReadU16(u64 ptr, u32 offset) {
        if (!LooksLikePtr(ptr)) return 0;
        return *reinterpret_cast<volatile u16*>(ptr + offset);
    }

    static inline const char* KnownSpeciesName(u16 species) {
        switch (species) {
            case 620: return "Mienshao";
            case 475: return "Gallade";
            case 534: return "Conkeldurr";
            case 448: return "Lucario";
            case 675: return "Pangoro";
            case 815: return "Cinderace";
            case 861: return "Grimmsnarl";
            default: return "";
        }
    }

    static inline bool IsKnownProbeSpecies(u16 species) {
        return species == 620 || species == 475 || species == 534 || species == 448 ||
               species == 675 || species == 815 || species == 861;
    }

    static inline u16 SpeciesFromEvalTarget(u64 eval_target) {
        // V13 latest log showed the target Pokémon species as a u16 at +0x70:
        //   target ptr ...6270 +0x70 = 0x032F = 815 Cinderace
        //   target ptr ...6A10 +0x70 = 0x035D = 861 Grimmsnarl
        //   target ptr ...C590 +0x70 = 0x01DB = 475 Gallade
        return ReadU16(eval_target, 0x70);
    }

    static inline void ScanKnownSpecies(const char* tag, u64 ptr, u32 hit, u32 bytes_to_scan) {
        if (!global_config.ai_bridge.score_survey_mode) return;
        if (!ShouldLog(hit)) return;
        if (!LooksLikePtr(ptr)) return;
        if (bytes_to_scan > 0x200) bytes_to_scan = 0x200;
        for (u32 off = 0; off + 1 < bytes_to_scan; off += 2) {
            const u16 species = ReadU16(ptr, off);
            if (IsKnownProbeSpecies(species)) {
                Logging.Log("[ai_bridge] species_probe %s hit=%u ptr=%016lx off=0x%x species=%u/%s\n",
                    tag, hit, ptr, off, static_cast<u32>(species), KnownSpeciesName(species));
            }
        }
    }

    static inline u64 LoHiPtr(u64 base, u32 offset) {
        if (!LooksLikePtr(base)) return 0;
        const u32 lo = *reinterpret_cast<volatile u32*>(base + offset + 0x0);
        const u32 hi = *reinterpret_cast<volatile u32*>(base + offset + 0x4);
        return (static_cast<u64>(hi) << 32) | static_cast<u64>(lo);
    }

    static inline void DumpCandidatePointerRefs(u64 state_ptr, u32 hit) {
        if (!global_config.ai_bridge.score_survey_mode) return;
        if (!ShouldLog(hit)) return;
        if (!LooksLikePtr(state_ptr)) return;
        const u32 offsets[] = {0x68, 0x70, 0x78, 0x80, 0xA8, 0xB0, 0x128, 0x130};
        const char* names[] = {"cs_68", "cs_70", "cs_78", "cs_80", "cs_A8", "cs_B0", "cs_128", "cs_130"};
        for (u32 i = 0; i < 8; i++) {
            const u64 ptr = LoHiPtr(state_ptr, offsets[i]);
            if (LooksLikePtr(ptr)) {
                Logging.Log("[ai_bridge] candidate_ptr %s hit=%u off=0x%x ptr=%016lx\n", names[i], hit, offsets[i], ptr);
                ScanKnownSpecies(names[i], ptr, hit, 0x120);
            }
        }
    }

    static inline u32 ClampSurveyWords(u32 words) {
        if (words == 0) return 0;
        if (words > 96) return 96;
        return words;
    }

    static inline void DumpRawWords(const char* tag, u64 ptr, u32 hit, u32 words) {
        if (!global_config.ai_bridge.score_survey_mode) return;
        if (!ShouldLog(hit)) return;
        if (!LooksLikePtr(ptr)) {
            Logging.Log("[ai_bridge] %s hit=%u ptr=%016lx skipped\n", tag, hit, ptr);
            return;
        }
        const u32 count = ClampSurveyWords(words);
        if (count == 0) return;
        volatile u32* w = reinterpret_cast<volatile u32*>(ptr);
        for (u32 i = 0; i < count; i += 8) {
            Logging.Log("[ai_bridge] %s hit=%u ptr=%016lx +%03x=%08x %08x %08x %08x %08x %08x %08x %08x\n",
                tag, hit, ptr, i * 4,
                w[i + 0], (i + 1 < count ? w[i + 1] : 0), (i + 2 < count ? w[i + 2] : 0), (i + 3 < count ? w[i + 3] : 0),
                (i + 4 < count ? w[i + 4] : 0), (i + 5 < count ? w[i + 5] : 0), (i + 6 < count ? w[i + 6] : 0), (i + 7 < count ? w[i + 7] : 0));
        }
    }


    struct MoveScoreRecord {
        u32 move_id;
        u32 target_low;
        s32 score_sum;
        s32 score_min;
        s32 score_max;
        u32 count;
    };

    inline MoveScoreRecord move_records[32] = {};
    inline u32 move_record_count = 0;
    inline bool move_records_consumed = false;
    inline u32 move_summary_generation = 0;

    static inline void ClearMoveScoreRecords() {
        for (u32 i = 0; i < 32; i++) {
            move_records[i] = {};
        }
        move_record_count = 0;
        move_records_consumed = false;
        move_summary_generation++;
    }

    static inline s32 SignedScore(u32 score) {
        return static_cast<s32>(score);
    }

    static inline u64 HiLoPtrFromOut(u64 out, u32 offset) {
        // The readback output structure logs address-like values as high-word,
        // low-word pairs in the u32 dump, e.g. 00000021 8ceb6270 -> 0x218ceb6270.
        const u32 hi = *reinterpret_cast<volatile u32*>(out + offset + 0x0);
        const u32 lo = *reinterpret_cast<volatile u32*>(out + offset + 0x4);
        return (static_cast<u64>(hi) << 32) | static_cast<u64>(lo);
    }

    static inline u32 MoveIdFromOut(u64 out) {
        // V8 logs showed the current evaluated move id at readback_out_raw +0xB0.
        // Examples observed in the user's log:
        //   0x00FC Fake Out
        //   0x0172 Close Combat
        //   0x01D5 Wide Guard
        //   0x00C5 Detect
        return *reinterpret_cast<volatile u32*>(out + 0xB0);
    }

    static inline u32 TargetKeyFromOut(u64 out) {
        // Use the low 32 bits of the apparent target/eval-object pointer as a
        // stable per-target key.  This is enough for comparing target variants
        // in one battle; we do not need it to be globally stable.
        return *reinterpret_cast<volatile u32*>(out + 0xA4);
    }

    static inline void RecordMoveScore(u32 move_id, u32 target_key, s32 score_delta) {
        if (move_id == 0 || move_id > 2000) return;

        for (u32 i = 0; i < move_record_count; i++) {
            MoveScoreRecord& r = move_records[i];
            if (r.move_id == move_id && r.target_low == target_key) {
                r.score_sum += score_delta;
                if (score_delta < r.score_min) r.score_min = score_delta;
                if (score_delta > r.score_max) r.score_max = score_delta;
                r.count++;
                return;
            }
        }

        if (move_record_count >= 32) return;
        MoveScoreRecord& r = move_records[move_record_count++];
        r.move_id = move_id;
        r.target_low = target_key;
        r.score_sum = score_delta;
        r.score_min = score_delta;
        r.score_max = score_delta;
        r.count = 1;
    }

    static inline void DumpMoveScoreSummary(u32 hit) {
        if (!global_config.ai_bridge.score_survey_mode) return;
        if (!ShouldLog(hit)) return;
        for (u32 i = 0; i < move_record_count; i++) {
            const MoveScoreRecord& r = move_records[i];
            Logging.Log("[ai_bridge] move_score_summary hit=%u rec=%u move=%u target=%08x sum=%d min=%d max=%d count=%u\n",
                hit, i, r.move_id, r.target_low, r.score_sum, r.score_min, r.score_max, r.count);
        }
    }

    static inline s32 BestMoveSummaryScore() {
        if (move_record_count == 0) return -999999;
        s32 best = -999999;
        for (u32 i = 0; i < move_record_count; i++) {
            const MoveScoreRecord& r = move_records[i];
            if (r.score_sum > best) best = r.score_sum;
        }
        return best;
    }

    static inline s32 WorstMoveSummaryScore() {
        if (move_record_count == 0) return 999999;
        s32 worst = 999999;
        for (u32 i = 0; i < move_record_count; i++) {
            const MoveScoreRecord& r = move_records[i];
            if (r.score_min < worst) worst = r.score_min;
            if (r.score_sum < worst) worst = r.score_sum;
        }
        return worst;
    }

    static inline bool MoveSummaryGateAllowsSwitch(const char* tag, u32 hit) {
        if (!global_config.ai_bridge.switch_gate_use_move_summary) return true;

        if (move_record_count < global_config.ai_bridge.switch_gate_min_move_records) {
            if (ShouldLog(hit)) {
                Logging.Log("[ai_bridge] %s move_summary_gate blocked: not_enough_records count=%u need=%u\n",
                    tag, move_record_count, global_config.ai_bridge.switch_gate_min_move_records);
            }
            return false;
        }

        const s32 best = BestMoveSummaryScore();
        const s32 worst = WorstMoveSummaryScore();
        bool has_negative = false;
        for (u32 i = 0; i < move_record_count; i++) {
            const MoveScoreRecord& r = move_records[i];
            if (r.score_min <= global_config.ai_bridge.switch_gate_negative_record_threshold ||
                r.score_sum <= global_config.ai_bridge.switch_gate_negative_record_threshold) {
                has_negative = true;
                break;
            }
        }

        if (global_config.ai_bridge.switch_gate_require_negative_record && !has_negative) {
            if (ShouldLog(hit)) {
                Logging.Log("[ai_bridge] %s move_summary_gate blocked: no_negative_record best=%d worst=%d count=%u threshold=%d\n",
                    tag, best, worst, move_record_count, global_config.ai_bridge.switch_gate_negative_record_threshold);
            }
            return false;
        }

        if (best > global_config.ai_bridge.switch_gate_best_move_max) {
            if (ShouldLog(hit)) {
                Logging.Log("[ai_bridge] %s move_summary_gate blocked: best_move_too_good best=%d max=%d worst=%d count=%u\n",
                    tag, best, global_config.ai_bridge.switch_gate_best_move_max, worst, move_record_count);
            }
            return false;
        }

        if (ShouldLog(hit)) {
            Logging.Log("[ai_bridge] %s move_summary_gate allowed: best=%d max=%d worst=%d count=%u negative=%u\n",
                tag, best, global_config.ai_bridge.switch_gate_best_move_max, worst, move_record_count, has_negative ? 1 : 0);
        }
        return true;
    }

    static inline void LogMoveEvalFromReadback(u64 out, u32 hit, u32 old_score, u32 old_enable) {
        if (!global_config.ai_bridge.score_survey_mode) return;

        const u32 move_id = MoveIdFromOut(out);
        const u32 target_key = TargetKeyFromOut(out);
        const u64 eval_target = HiLoPtrFromOut(out, 0xA0);
        const u64 move_obj = HiLoPtrFromOut(out, 0xA8);
        const u64 actor_or_side = (static_cast<u64>(*reinterpret_cast<volatile u32*>(out + 0xB8)) << 32)
            | static_cast<u64>(*reinterpret_cast<volatile u32*>(out + 0xB4));
        const s32 score_delta = SignedScore(old_score);

        // V11: In V10, summaries were effectively global and also stopped
        // updating after max_log_hits.  The candidate gate kept seeing an old
        // best=+6 forever.  Now the score survey continues recording even when
        // log output is capped, and it resets the summary after the candidate
        // gate has consumed one complete scoring window.
        if (move_records_consumed) {
            ClearMoveScoreRecords();
        }
        RecordMoveScore(move_id, target_key, score_delta);

        if (!ShouldLog(hit)) return;

        const u16 target_species = SpeciesFromEvalTarget(eval_target);
        Logging.Log("[ai_bridge] move_eval hit=%u gen=%u move=%u target=%08x target_species=%u/%s score_delta=%d enable=%u eval_target=%016lx move_obj=%016lx actor_or_side=%016lx\n",
            hit, move_summary_generation, move_id, target_key,
            static_cast<u32>(target_species), KnownSpeciesName(target_species),
            score_delta, old_enable, eval_target, move_obj, actor_or_side);

        // V14: target species mapping.  The eval_target object now directly
        // tells us which Pokémon the AI is evaluating against.  This is the
        // first reliable runtime species read and is the basis for matchup-aware
        // switch scoring.
        ScanKnownSpecies("eval_target", eval_target, hit, 0x120);
        ScanKnownSpecies("move_obj", move_obj, hit, 0x180);

        // V13: optional deep survey of the structures that likely identify
        // the actor/side, move object, and target/eval object.  This is the
        // next step toward real bench switch-in scoring: we need to locate
        // active species/party slot/side from these objects instead of only
        // seeing move IDs and candidate IDs.
        if (global_config.ai_bridge.score_survey_dump_state) {
            DumpRawWords("eval_target_raw", eval_target, hit, global_config.ai_bridge.score_survey_words);
            DumpRawWords("move_obj_raw", move_obj, hit, global_config.ai_bridge.score_survey_words);
            DumpRawWords("actor_side_raw", actor_or_side, hit, global_config.ai_bridge.score_survey_words);
        }

        DumpMoveScoreSummary(hit);
    }

    static inline void ForceCandidateEntryIfEnabled(u64 state_ptr, const char* tag, u32 hit) {
        if (!global_config.ai_bridge.force_candidate_entry) return;
        if (hit != 1) return; // legacy one-shot injection, first hit only for safety
        if (state_ptr < 0x100000) return;

        const u32 slot = global_config.ai_bridge.force_candidate_slot;
        if (slot >= 16) return;
        const u64 entry = state_ptr + 0xC8 + (static_cast<u64>(slot) * 8);
        *reinterpret_cast<volatile u8*>(entry + 0x0) = static_cast<u8>(global_config.ai_bridge.force_candidate_action_id & 0xFF);
        *reinterpret_cast<volatile u8*>(entry + 0x1) = global_config.ai_bridge.force_candidate_enable ? 1 : 0;
        *reinterpret_cast<volatile u32*>(entry + 0x4) = global_config.ai_bridge.force_candidate_score;

        // Ensure candidate count reaches this slot.
        volatile u8* count = reinterpret_cast<volatile u8*>(state_ptr + 0xC5);
        if (*count <= slot) *count = static_cast<u8>(slot + 1);

        Logging.Log("[ai_bridge] %s force_candidate slot=%u action=%u enable=%u score=%u\n",
            tag, slot, global_config.ai_bridge.force_candidate_action_id,
            global_config.ai_bridge.force_candidate_enable ? 1 : 0,
            global_config.ai_bridge.force_candidate_score);
    }

    static inline void ResetSwitchPolicyIfNeeded(u64 state_ptr) {
        if (AIBridge::policy_state_ptr == state_ptr) return;
        AIBridge::policy_state_ptr = state_ptr;
        AIBridge::policy_forced_total = 0;
        AIBridge::policy_forced_action2 = 0;
        AIBridge::policy_forced_action3 = 0;
        AIBridge::policy_forced_other = 0;
        AIBridge::policy_cooldown = 0;
        AIBridge::policy_last_preferred_action = 3;
    }

    static inline u32& ForcedCountForAction(u32 action_id) {
        if (action_id == 2) return AIBridge::policy_forced_action2;
        if (action_id == 3) return AIBridge::policy_forced_action3;
        return AIBridge::policy_forced_other;
    }

    static inline u32 TargetScoreForAction(u32 action_id) {
        if (action_id == 2 && global_config.ai_bridge.switch_action2_score != 0) {
            return global_config.ai_bridge.switch_action2_score;
        }
        if (action_id == 3 && global_config.ai_bridge.switch_action3_score != 0) {
            return global_config.ai_bridge.switch_action3_score;
        }
        return global_config.ai_bridge.switch_score;
    }

    static inline bool NativeScoreAllowed(u32 native_score) {
        if (native_score < global_config.ai_bridge.switch_min_native_score) return false;
        if (global_config.ai_bridge.switch_max_native_score != 0 &&
            native_score > global_config.ai_bridge.switch_max_native_score) return false;
        return true;
    }

    static inline u32 CountMatchingSwitchCandidates(u64 state_ptr, u32 count, u32 min_action, u32 max_action) {
        u32 matches = 0;
        for (u32 i = 0; i < count; i++) {
            const u64 entry = state_ptr + 0xC8 + (static_cast<u64>(i) * 8);
            const u32 action_id = static_cast<u32>(*reinterpret_cast<volatile u8*>(entry + 0x0));
            const u32 native_score = *reinterpret_cast<volatile u32*>(entry + 0x4);
            if (action_id < min_action || action_id > max_action) continue;
            if (!NativeScoreAllowed(native_score)) continue;
            matches++;
        }
        return matches;
    }

    static inline bool PreferredActionAllowed(u32 action_id, u32 preferred) {
        if (preferred == 0) return true;
        return action_id == preferred;
    }

    static inline u32 PickPreferredActionForThisHit() {
        const u32 configured = global_config.ai_bridge.switch_preferred_action;
        if (configured == 2 || configured == 3) return configured;
        if (!global_config.ai_bridge.switch_alternate_actions) return 2;
        AIBridge::policy_last_preferred_action = (AIBridge::policy_last_preferred_action == 2) ? 3 : 2;
        return AIBridge::policy_last_preferred_action;
    }

    static inline void CaptureDeferredCandidateState(u64 state_ptr, u32 score_hit) {
        if (global_config.ai_bridge.switch_policy_mode != 5) return;
        if (state_ptr < 0x100000) return;

        volatile u8* b = reinterpret_cast<volatile u8*>(state_ptr);
        const u32 count = ClampCandidateCount(static_cast<u32>(b[0xC5]));
        if (count == 0) return;

        const u32 min_action = global_config.ai_bridge.force_existing_action_min;
        const u32 max_action = global_config.ai_bridge.force_existing_action_max;
        const u32 matching = CountMatchingSwitchCandidates(state_ptr, count, min_action, max_action);
        if (matching == 0) return;

        deferred_candidate_state_ptr = state_ptr;
        deferred_candidate_score_hit = score_hit;
        deferred_candidate_ready = true;
        deferred_policy_applied_this_window = false;

        if (ShouldLog(score_hit)) {
            Logging.Log("[ai_bridge] deferred_capture hit=%u state=%016lx count=%u matching=%u\n",
                score_hit, state_ptr, count, matching);
        }
    }

    static inline void ApplySwitchPolicyIfEnabled(u64 state_ptr, const char* tag, u32 hit) {
        const u32 mode = global_config.ai_bridge.switch_policy_mode;
        if (mode == 0) return;
        if (hit < global_config.ai_bridge.switch_min_hit) return;
        if (state_ptr < 0x100000) return;

        const bool summary_ready_for_gate = global_config.ai_bridge.switch_gate_use_move_summary &&
            move_record_count >= global_config.ai_bridge.switch_gate_min_move_records;
        const bool gate_allows_switch = MoveSummaryGateAllowsSwitch(tag, hit);
        if (summary_ready_for_gate) {
            // Mark this scoring window consumed.  The next readback/move_eval
            // starts a new summary generation.  This prevents a good move from
            // turn 1 from blocking switches forever on later turns.
            move_records_consumed = true;
        }
        if (!gate_allows_switch) return;

        ResetSwitchPolicyIfNeeded(state_ptr);

        if (AIBridge::policy_cooldown != 0) {
            AIBridge::policy_cooldown--;
            return;
        }

        if (global_config.ai_bridge.switch_max_forces_per_battle != 0 &&
            AIBridge::policy_forced_total >= global_config.ai_bridge.switch_max_forces_per_battle) {
            return;
        }

        volatile u8* b = reinterpret_cast<volatile u8*>(state_ptr);
        const u32 count = ClampCandidateCount(static_cast<u32>(b[0xC5]));
        if (count == 0) return;

        const u32 min_action = global_config.ai_bridge.force_existing_action_min;
        const u32 max_action = global_config.ai_bridge.force_existing_action_max;
        const u32 matching = CountMatchingSwitchCandidates(state_ptr, count, min_action, max_action);

        // Conservative default: only act once both active-slot switch candidates
        // are present.  This avoids enabling a lone switch candidate every time
        // the table is partially rebuilt.
        if (global_config.ai_bridge.switch_require_candidate_count != 0 &&
            matching < global_config.ai_bridge.switch_require_candidate_count) {
            return;
        }

        const u32 preferred_action = PickPreferredActionForThisHit();
        const u32 max_changed_this_hit = global_config.ai_bridge.switch_max_candidates_per_hit == 0
            ? 1
            : global_config.ai_bridge.switch_max_candidates_per_hit;
        u32 changed = 0;

        for (u32 pass = 0; pass < 2 && changed < max_changed_this_hit; pass++) {
            for (u32 i = 0; i < count && changed < max_changed_this_hit; i++) {
                const u64 entry = state_ptr + 0xC8 + (static_cast<u64>(i) * 8);
                const u32 action_id = static_cast<u32>(*reinterpret_cast<volatile u8*>(entry + 0x0));
                if (action_id < min_action || action_id > max_action) continue;
                if (pass == 0 && !PreferredActionAllowed(action_id, preferred_action)) continue;

                volatile u8* enabled = reinterpret_cast<volatile u8*>(entry + 0x1);
                volatile u32* score = reinterpret_cast<volatile u32*>(entry + 0x4);
                const u32 native_score = *score;
                if (!NativeScoreAllowed(native_score)) continue;

                u32& action_count = ForcedCountForAction(action_id);
                if (global_config.ai_bridge.switch_max_forces_per_action != 0 &&
                    action_count >= global_config.ai_bridge.switch_max_forces_per_action) {
                    continue;
                }

                const u32 target_score = TargetScoreForAction(action_id);
                bool did_change = false;

                // mode 1: enable only; keep native score.
                // mode 2: enable and raise to target_score only if lower.
                // mode 3: enable and set target_score every time.
                // mode 4: V7 conservative mode; enable at most one candidate
                //         when the native switch score is in the allowed range.
                if (!*enabled) {
                    *enabled = 1;
                    did_change = true;
                }

                if (!global_config.ai_bridge.switch_native_score_only) {
                    if ((mode == 2 || mode == 4 || mode == 5) && *score < target_score) {
                        *score = target_score;
                        did_change = true;
                    } else if (mode >= 3 && mode != 4 && mode != 5 && *score != target_score) {
                        *score = target_score;
                        did_change = true;
                    }
                }

                if (did_change) {
                    changed++;
                    AIBridge::policy_forced_total++;
                    action_count++;
                    if (global_config.ai_bridge.switch_disable_after_force) {
                        for (u32 j = i + 1; j < count; j++) {
                            const u64 other = state_ptr + 0xC8 + (static_cast<u64>(j) * 8);
                            const u32 other_action = static_cast<u32>(*reinterpret_cast<volatile u8*>(other + 0x0));
                            if (other_action >= min_action && other_action <= max_action) {
                                *reinterpret_cast<volatile u8*>(other + 0x1) = 0;
                            }
                        }
                    }
                    if (global_config.ai_bridge.switch_cooldown_hits != 0) {
                        AIBridge::policy_cooldown = global_config.ai_bridge.switch_cooldown_hits;
                    }
                }
            }
        }

        if (changed != 0 && ShouldLog(hit)) {
            Logging.Log("[ai_bridge] %s switch_policy_v12 mode=%u changed=%u total=%u a2=%u a3=%u other=%u cooldown=%u score=%u matching=%u preferred=%u native_only=%u\n",
                tag, mode, changed,
                AIBridge::policy_forced_total,
                AIBridge::policy_forced_action2,
                AIBridge::policy_forced_action3,
                AIBridge::policy_forced_other,
                AIBridge::policy_cooldown,
                global_config.ai_bridge.switch_score,
                matching,
                preferred_action,
                global_config.ai_bridge.switch_native_score_only ? 1 : 0);
            DumpCandidateTable("candidate_score_after_switch_policy_v12", state_ptr, hit);
        }
    }

    static inline void ApplyDeferredSwitchPolicyIfReady(u32 output_hit) {
        if (global_config.ai_bridge.switch_policy_mode != 5) return;
        if (!deferred_candidate_ready || deferred_policy_applied_this_window) return;
        if (deferred_candidate_state_ptr < 0x100000) return;

        // Wait until the current move scoring window has enough records.
        // Existing config keys control the threshold: switch_gate_min_move_records
        // and switch_min_hit.  In V12, switch_min_hit is interpreted as an
        // output/readback hit threshold for the deferred application.
        if (output_hit < global_config.ai_bridge.switch_min_hit) return;
        if (global_config.ai_bridge.switch_gate_use_move_summary &&
            move_record_count < global_config.ai_bridge.switch_gate_min_move_records) {
            if (ShouldLog(output_hit)) {
                Logging.Log("[ai_bridge] deferred_switch blocked: not_enough_records output_hit=%u records=%u need=%u\n",
                    output_hit, move_record_count, global_config.ai_bridge.switch_gate_min_move_records);
            }
            return;
        }

        if (ShouldLog(output_hit)) {
            Logging.Log("[ai_bridge] deferred_switch attempting output_hit=%u candidate_hit=%u state=%016lx records=%u best=%d worst=%d\n",
                output_hit, deferred_candidate_score_hit, deferred_candidate_state_ptr,
                move_record_count, BestMoveSummaryScore(), WorstMoveSummaryScore());
        }

        ApplySwitchPolicyIfEnabled(deferred_candidate_state_ptr, "deferred_candidate_score", output_hit);
        deferred_policy_applied_this_window = true;
    }

    static inline void ForceExistingCandidatesIfEnabled(u64 state_ptr, const char* tag, u32 hit) {
        if (!global_config.ai_bridge.force_existing_candidates) return;
        if (state_ptr < 0x100000) return;

        volatile u8* b = reinterpret_cast<volatile u8*>(state_ptr);
        const u32 count = ClampCandidateCount(static_cast<u32>(b[0xC5]));
        if (count == 0) return;

        const u32 min_action = global_config.ai_bridge.force_existing_action_min;
        const u32 max_action = global_config.ai_bridge.force_existing_action_max;
        u32 changed = 0;

        for (u32 i = 0; i < count; i++) {
            const u64 entry = state_ptr + 0xC8 + (static_cast<u64>(i) * 8);
            const u32 action_id = static_cast<u32>(*reinterpret_cast<volatile u8*>(entry + 0x0));
            if (action_id < min_action || action_id > max_action) continue;

            *reinterpret_cast<volatile u8*>(entry + 0x1) = global_config.ai_bridge.force_existing_enable ? 1 : 0;
            *reinterpret_cast<volatile u32*>(entry + 0x4) = global_config.ai_bridge.force_existing_score;
            changed++;
        }

        if (changed != 0 && ShouldLog(hit)) {
            Logging.Log("[ai_bridge] %s force_existing changed=%u count=%u action_range=%u-%u enable=%u score=%u\n",
                tag, changed, count, min_action, max_action,
                global_config.ai_bridge.force_existing_enable ? 1 : 0,
                global_config.ai_bridge.force_existing_score);
            DumpCandidateTable("candidate_score_after_force_existing", state_ptr, hit);
        }
    }

    static inline void ForceFinalSelectionIfEnabled(u64 state_ptr, const char* tag, u32 hit) {
        if (!global_config.ai_bridge.force_final_selection) return;
        if (state_ptr < 0x100000) return;

        *reinterpret_cast<volatile u8*>(state_ptr + 0xF8) = 1;
        *reinterpret_cast<volatile u32*>(state_ptr + 0xFC) = global_config.ai_bridge.force_final_score;
        *reinterpret_cast<volatile u32*>(state_ptr + 0x100) = global_config.ai_bridge.force_final_action_id;

        if (ShouldLog(hit)) {
            Logging.Log("[ai_bridge] %s force_final action=%u score=%u\n",
                tag, global_config.ai_bridge.force_final_action_id,
                global_config.ai_bridge.force_final_score);
        }
    }

    // V12 one-shot action mapper.  This is for identifying what action=2 and
    // action=3 actually mean in live battle: which active slot switches out and
    // what replacement logic chooses.  It deliberately forces only a limited
    // number of matching candidates, unlike the old v5 force-existing proof.
    inline u32 action_probe_uses = 0;

    static inline void ApplyActionProbeIfEnabled(u64 state_ptr, const char* tag, u32 hit) {
        if (!global_config.ai_bridge.action_probe_enabled) return;
        if (action_probe_uses >= global_config.ai_bridge.action_probe_max_uses) return;
        if (state_ptr < 0x100000) return;

        volatile u8* b = reinterpret_cast<volatile u8*>(state_ptr);
        const u32 count = ClampCandidateCount(static_cast<u32>(b[0xC5]));
        const u32 wanted = global_config.ai_bridge.action_probe_action_id;
        for (u32 i = 0; i < count && i < 16; i++) {
            const u64 entry = state_ptr + 0xC8 + (static_cast<u64>(i) * 8);
            volatile u8* eb = reinterpret_cast<volatile u8*>(entry);
            volatile u32* score_ptr = reinterpret_cast<volatile u32*>(entry + 0x4);
            const u32 action_id = static_cast<u32>(eb[0]);
            if (action_id != wanted) continue;

            const u32 old_enabled = static_cast<u32>(eb[1]);
            const u32 old_score = *score_ptr;
            eb[1] = 1;
            *score_ptr = global_config.ai_bridge.action_probe_score;
            action_probe_uses++;
            Logging.Log("[ai_bridge] action_probe %s hit=%u use=%u/%u slot=%u action=%u old_enabled=%u old_score=%u new_score=%u\n",
                tag, hit, action_probe_uses, global_config.ai_bridge.action_probe_max_uses,
                i, action_id, old_enabled, old_score, global_config.ai_bridge.action_probe_score);
            return;
        }
    }

}

HOOK_DEFINE_INLINE(AIBridgeOutputReadbackPostProbe) {
    static void Callback(exl::hook::nx64::InlineCtx* ctx) {
        if (!global_config.initialized || !global_config.ai_bridge.active) return;
        AIBridge::output_hits++;

        // In this function x19 is the output object x1 passed to 0x7D4B80.
        const u64 out = ctx->X[19];
        if (out >= 0x100000) {
            volatile u32* score_ptr = reinterpret_cast<volatile u32*>(out + 0x0);
            volatile u8* enable_ptr = reinterpret_cast<volatile u8*>(out + 0x4);
            const u32 old_score = *score_ptr;
            const u32 old_enable = *enable_ptr;

            if (AIBridge::ShouldLog(AIBridge::output_hits)) {
                Logging.Log("[ai_bridge] readback_post hit=%u out=%016lx score=%u enable=%u x0=%016lx x19=%016lx x30=%016lx\n",
                    AIBridge::output_hits, out, old_score, old_enable, ctx->X[0], ctx->X[19], ctx->X[30]);
            }

            AIBridge::LogMoveEvalFromReadback(out, AIBridge::output_hits, old_score, old_enable);
            AIBridge::ApplyDeferredSwitchPolicyIfReady(AIBridge::output_hits);

            if (global_config.ai_bridge.score_survey_mode) {
                if (global_config.ai_bridge.score_survey_dump_readback_out) {
                    AIBridge::DumpRawWords("readback_out_raw", out, AIBridge::output_hits, global_config.ai_bridge.score_survey_words);
                }
                if (global_config.ai_bridge.score_survey_dump_readback_x0) {
                    AIBridge::DumpRawWords("readback_x0_raw", ctx->X[0], AIBridge::output_hits, global_config.ai_bridge.score_survey_words);
                }
            }

            if (global_config.ai_bridge.force_output_score != 0) {
                *score_ptr = global_config.ai_bridge.force_output_score;
            }
            if (global_config.ai_bridge.force_pokechange_enable >= 0) {
                *enable_ptr = static_cast<u8>(global_config.ai_bridge.force_pokechange_enable & 1);
            }
        }
    }
};

HOOK_DEFINE_INLINE(AIBridgeCandidateListPostProbe) {
    static void Callback(exl::hook::nx64::InlineCtx* ctx) {
        if (!global_config.initialized || !global_config.ai_bridge.active) return;
        AIBridge::list_hits++;
        // x19 is the candidate-selection state object in this function.
        AIBridge::DumpCandidateTable("candidate_list", ctx->X[19], AIBridge::list_hits);
        AIBridge::ForceCandidateEntryIfEnabled(ctx->X[19], "candidate_list", AIBridge::list_hits);
    }
};

HOOK_DEFINE_INLINE(AIBridgeCandidateScorePostProbe) {
    static void Callback(exl::hook::nx64::InlineCtx* ctx) {
        if (!global_config.initialized || !global_config.ai_bridge.active) return;
        AIBridge::score_hits++;
        AIBridge::DumpCandidateTable("candidate_score", ctx->X[19], AIBridge::score_hits);
        if (global_config.ai_bridge.score_survey_dump_state) {
            AIBridge::DumpRawWords("candidate_state_raw", ctx->X[19], AIBridge::score_hits, global_config.ai_bridge.score_survey_words);
            AIBridge::DumpCandidatePointerRefs(ctx->X[19], AIBridge::score_hits);
        }
        // V12 one-shot action mapper.  Prefer this over the old force-existing proof
        // when trying to identify action IDs.
        AIBridge::ApplyActionProbeIfEnabled(ctx->X[19], "candidate_score", AIBridge::score_hits);

        // V5: if the existing candidate rows are switch candidates, this turns
        // them on and gives them a dominant score every time they appear.
        AIBridge::ForceExistingCandidatesIfEnabled(ctx->X[19], "candidate_score", AIBridge::score_hits);
        if (global_config.ai_bridge.switch_policy_mode == 5) {
            AIBridge::CaptureDeferredCandidateState(ctx->X[19], AIBridge::score_hits);
        } else {
            AIBridge::ApplySwitchPolicyIfEnabled(ctx->X[19], "candidate_score", AIBridge::score_hits);
        }
        AIBridge::ForceCandidateEntryIfEnabled(ctx->X[19], "candidate_score", AIBridge::score_hits);
    }
};

HOOK_DEFINE_INLINE(AIBridgeFinalSelectionPostProbe) {
    static void Callback(exl::hook::nx64::InlineCtx* ctx) {
        if (!global_config.initialized || !global_config.ai_bridge.active) return;
        AIBridge::final_hits++;
        AIBridge::DumpCandidateTable("final_selection", ctx->X[19], AIBridge::final_hits);
        AIBridge::ForceFinalSelectionIfEnabled(ctx->X[19], "final_selection", AIBridge::final_hits);
        // Log again after the force so we can confirm memory changed.
        if (global_config.ai_bridge.force_final_selection) {
            AIBridge::DumpCandidateTable("final_selection_after_force", ctx->X[19], AIBridge::final_hits);
        }
    }
};

void install_ai_bridge_patch() {
    if (AIBridge::installed) return;
    if (!global_config.initialized) return;
    if (!global_config.ai_bridge.active) return;

    if (!global_config.ai_bridge.install_candidate_hooks) {
        Logging.Log("[ai_bridge] active but install_candidate_hooks=false; no hooks installed\n");
        AIBridge::installed = true;
        return;
    }

#ifdef VERSION_SHIELD
    Logging.Log("[ai_bridge] Shield offsets are not mapped. No AI bridge hooks installed.\n");
#else
    if (global_config.ai_bridge.hook_output_readback_post) {
        AIBridgeOutputReadbackPostProbe::InstallAtOffset(global_config.ai_bridge.output_readback_post_offset);
        Logging.Log("[ai_bridge] installed readback_post at 0x%lx\n", global_config.ai_bridge.output_readback_post_offset);
    }
    if (global_config.ai_bridge.hook_candidate_list_post) {
        AIBridgeCandidateListPostProbe::InstallAtOffset(global_config.ai_bridge.candidate_list_post_offset);
        Logging.Log("[ai_bridge] installed candidate_list_post at 0x%lx\n", global_config.ai_bridge.candidate_list_post_offset);
    }
    if (global_config.ai_bridge.hook_candidate_score_post) {
        AIBridgeCandidateScorePostProbe::InstallAtOffset(global_config.ai_bridge.candidate_score_post_offset);
        Logging.Log("[ai_bridge] installed candidate_score_post at 0x%lx\n", global_config.ai_bridge.candidate_score_post_offset);
    }
    if (global_config.ai_bridge.hook_final_selection_post) {
        AIBridgeFinalSelectionPostProbe::InstallAtOffset(global_config.ai_bridge.final_selection_post_offset);
        Logging.Log("[ai_bridge] installed final_selection_post at 0x%lx\n", global_config.ai_bridge.final_selection_post_offset);
    }
#endif

    AIBridge::installed = true;
}
