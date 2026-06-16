#pragma once
#include "tomlplusplus/toml.hpp"

struct PatchConfig {
    bool initialized = false;

    struct {
        bool active;
    } underworld;

    struct {
        bool active;
        u32 boosted_percentage;
        std::string sound;
        std::string shiny_ptcl;
        bool show_aura_for_brilliants;
        bool include_battle_sounds;
        bool play_sound_for_following;
        bool show_ptcl_for_following;
        bool show_message_box;
    } overworld_shiny;

    struct {
        bool active;
    } randomizer;

    struct {
        bool active;
        float adjustment_speed;
        float min_pitch;
        float max_pitch;
        float min_distance;
        float max_distance;
    } camera_tweaks;

    struct {
        bool active;
        bool fully;
    } uncap_level;

    struct {
        bool active;
        bool disable_terrain_culling;
        bool always_use_extended_camera;
    } freecam;

    struct {
        bool active;
        s32 maximum_spawns;
    } glimwood_overworld;

    struct {
        bool active;
    } synchro_mode;

    struct {
        bool active;
    } extended_following;

    struct {
        bool active;
        bool keep_chain_after_brilliant;
    } fishing_tweaks;

    struct {
        bool active;
    } dex_animations;

    struct {
        bool active;
        bool install_candidate_hooks;
        u64 output_readback_post_offset;
        u64 candidate_list_post_offset;
        u64 candidate_score_post_offset;
        u64 final_selection_post_offset;
        bool hook_output_readback_post;
        bool hook_candidate_list_post;
        bool hook_candidate_score_post;
        bool hook_final_selection_post;
        bool log_hits;
        u32 max_log_hits;
        u32 candidate_dump_count;
        u32 force_output_score;
        s32 force_pokechange_enable;
        bool force_final_selection;
        u32 force_final_action_id;
        u32 force_final_score;
        bool force_candidate_entry;
        u32 force_candidate_slot;
        u32 force_candidate_action_id;
        u32 force_candidate_score;
        bool force_candidate_enable;
        bool force_existing_candidates;
        u32 force_existing_action_min;
        u32 force_existing_action_max;
        u32 force_existing_score;
        bool force_existing_enable;
        u32 switch_policy_mode;
        u32 switch_min_hit;
        u32 switch_score;
        u32 switch_max_forces_per_battle;
        u32 switch_max_forces_per_action;
        u32 switch_cooldown_hits;
        u32 switch_action2_score;
        u32 switch_action3_score;
        u32 switch_max_candidates_per_hit;
        u32 switch_require_candidate_count;
        u32 switch_min_native_score;
        u32 switch_max_native_score;
        u32 switch_preferred_action;
        bool switch_alternate_actions;
        bool switch_native_score_only;
        bool switch_disable_after_force;
        bool switch_gate_use_move_summary;
        s32 switch_gate_best_move_max;
        bool switch_gate_require_negative_record;
        s32 switch_gate_negative_record_threshold;
        u32 switch_gate_min_move_records;
        bool score_survey_mode;
        bool score_survey_dump_state;
        bool score_survey_dump_readback_x0;
        bool score_survey_dump_readback_out;
        u32 score_survey_words;
        bool action_probe_enabled;
        u32 action_probe_action_id;
        u32 action_probe_score;
        u32 action_probe_max_uses;
    } ai_bridge;

    void from_table(toml::parse_result &table) {
        underworld.active = table["underworld"]["active"].value_or(false);

        overworld_shiny.active = table["overworld_shiny"]["active"].value_or(false);
        overworld_shiny.boosted_percentage = table["overworld_shiny"]["boosted_percentage"].value_or(0);
        overworld_shiny.sound = table["overworld_shiny"]["sound"].value_or("Play_Camp_Cooking_Explosion");
        overworld_shiny.shiny_ptcl = table["overworld_shiny"]["shiny_ptcl"].value_or("bin/field/effect/particle/ef_cyc_aura/ef_cyc_aura_rare_bk.ptcl");
        overworld_shiny.show_aura_for_brilliants = table["overworld_shiny"]["show_aura_for_brilliants"].value_or(true);
        overworld_shiny.show_message_box = table["overworld_shiny"]["show_message_box"].value_or(false);
        overworld_shiny.include_battle_sounds = table["overworld_shiny"]["include_battle_sounds"].value_or(true);
        overworld_shiny.play_sound_for_following = table["overworld_shiny"]["play_sound_for_following"].value_or(false);
        overworld_shiny.show_ptcl_for_following = table["overworld_shiny"]["show_ptcl_for_following"].value_or(true);

        randomizer.active = table["randomizer"]["active"].value_or(false);

        camera_tweaks.active = table["camera_tweaks"]["active"].value_or(false);
        camera_tweaks.adjustment_speed = table["camera_tweaks"]["adjustment_speed"].value_or(3.0);
        camera_tweaks.min_pitch = table["camera_tweaks"]["min_pitch"].value_or(-20.0);
        camera_tweaks.max_pitch = table["camera_tweaks"]["max_pitch"].value_or(-5.0);
        camera_tweaks.min_distance = table["camera_tweaks"]["min_distance"].value_or(370.0);
        camera_tweaks.max_distance = table["camera_tweaks"]["max_distance"].value_or(800.0);

        uncap_level.active = table["uncap_level"]["active"].value_or(false);
        uncap_level.fully = table["uncap_level"]["fully"].value_or(true);

        freecam.active = table["freecam"]["active"].value_or(false);
        freecam.disable_terrain_culling = table["freecam"]["disable_terrain_culling"].value_or(true);
        freecam.always_use_extended_camera = table["freecam"]["always_use_extended_camera"].value_or(false);

        glimwood_overworld.active = table["glimwood_overworld"]["active"].value_or(false);
        glimwood_overworld.maximum_spawns = table["glimwood_overworld"]["maximum_spawns"].value_or(1);

        synchro_mode.active = table["synchro_mode"]["active"].value_or(false);

        extended_following.active = table["extended_following"]["active"].value_or(false);

        fishing_tweaks.active = table["fishing_tweaks"]["active"].value_or(false);
        fishing_tweaks.keep_chain_after_brilliant = table["fishing_tweaks"]["keep_chain_after_brilliant"].value_or(true);

        dex_animations.active = table["dex_animations"]["active"].value_or(false);

        ai_bridge.active = table["ai_bridge"]["active"].value_or(false);
        ai_bridge.install_candidate_hooks = table["ai_bridge"]["install_candidate_hooks"].value_or(false);
        ai_bridge.output_readback_post_offset = static_cast<u64>(table["ai_bridge"]["output_readback_post_offset"].value_or(8211420ull));
        ai_bridge.candidate_list_post_offset = static_cast<u64>(table["ai_bridge"]["candidate_list_post_offset"].value_or(9342476ull));
        ai_bridge.candidate_score_post_offset = static_cast<u64>(table["ai_bridge"]["candidate_score_post_offset"].value_or(9342576ull));
        ai_bridge.final_selection_post_offset = static_cast<u64>(table["ai_bridge"]["final_selection_post_offset"].value_or(9342816ull));
        ai_bridge.hook_output_readback_post = table["ai_bridge"]["hook_output_readback_post"].value_or(false);
        ai_bridge.hook_candidate_list_post = table["ai_bridge"]["hook_candidate_list_post"].value_or(false);
        ai_bridge.hook_candidate_score_post = table["ai_bridge"]["hook_candidate_score_post"].value_or(false);
        ai_bridge.hook_final_selection_post = table["ai_bridge"]["hook_final_selection_post"].value_or(false);
        ai_bridge.log_hits = table["ai_bridge"]["log_hits"].value_or(false);
        ai_bridge.max_log_hits = table["ai_bridge"]["max_log_hits"].value_or(64);
        ai_bridge.candidate_dump_count = table["ai_bridge"]["candidate_dump_count"].value_or(8);
        ai_bridge.force_output_score = table["ai_bridge"]["force_output_score"].value_or(0);
        ai_bridge.force_pokechange_enable = table["ai_bridge"]["force_pokechange_enable"].value_or(-1);
        ai_bridge.force_final_selection = table["ai_bridge"]["force_final_selection"].value_or(false);
        ai_bridge.force_final_action_id = table["ai_bridge"]["force_final_action_id"].value_or(0);
        ai_bridge.force_final_score = table["ai_bridge"]["force_final_score"].value_or(1000000);
        ai_bridge.force_candidate_entry = table["ai_bridge"]["force_candidate_entry"].value_or(false);
        ai_bridge.force_candidate_slot = table["ai_bridge"]["force_candidate_slot"].value_or(0);
        ai_bridge.force_candidate_action_id = table["ai_bridge"]["force_candidate_action_id"].value_or(0);
        ai_bridge.force_candidate_score = table["ai_bridge"]["force_candidate_score"].value_or(1000000);
        ai_bridge.force_candidate_enable = table["ai_bridge"]["force_candidate_enable"].value_or(true);
        ai_bridge.force_existing_candidates = table["ai_bridge"]["force_existing_candidates"].value_or(false);
        ai_bridge.force_existing_action_min = table["ai_bridge"]["force_existing_action_min"].value_or(0);
        ai_bridge.force_existing_action_max = table["ai_bridge"]["force_existing_action_max"].value_or(255);
        ai_bridge.force_existing_score = table["ai_bridge"]["force_existing_score"].value_or(1000000);
        ai_bridge.force_existing_enable = table["ai_bridge"]["force_existing_enable"].value_or(true);
        ai_bridge.switch_policy_mode = table["ai_bridge"]["switch_policy_mode"].value_or(0);
        ai_bridge.switch_min_hit = table["ai_bridge"]["switch_min_hit"].value_or(1);
        ai_bridge.switch_score = table["ai_bridge"]["switch_score"].value_or(350);
        ai_bridge.switch_max_forces_per_battle = table["ai_bridge"]["switch_max_forces_per_battle"].value_or(6);
        ai_bridge.switch_max_forces_per_action = table["ai_bridge"]["switch_max_forces_per_action"].value_or(3);
        ai_bridge.switch_cooldown_hits = table["ai_bridge"]["switch_cooldown_hits"].value_or(0);
        ai_bridge.switch_action2_score = table["ai_bridge"]["switch_action2_score"].value_or(0);
        ai_bridge.switch_action3_score = table["ai_bridge"]["switch_action3_score"].value_or(0);
        ai_bridge.switch_max_candidates_per_hit = table["ai_bridge"]["switch_max_candidates_per_hit"].value_or(1);
        ai_bridge.switch_require_candidate_count = table["ai_bridge"]["switch_require_candidate_count"].value_or(2);
        ai_bridge.switch_min_native_score = table["ai_bridge"]["switch_min_native_score"].value_or(0);
        ai_bridge.switch_max_native_score = table["ai_bridge"]["switch_max_native_score"].value_or(150);
        ai_bridge.switch_preferred_action = table["ai_bridge"]["switch_preferred_action"].value_or(0);
        ai_bridge.switch_alternate_actions = table["ai_bridge"]["switch_alternate_actions"].value_or(true);
        ai_bridge.switch_native_score_only = table["ai_bridge"]["switch_native_score_only"].value_or(false);
        ai_bridge.switch_disable_after_force = table["ai_bridge"]["switch_disable_after_force"].value_or(false);
        ai_bridge.switch_gate_use_move_summary = table["ai_bridge"]["switch_gate_use_move_summary"].value_or(false);
        ai_bridge.switch_gate_best_move_max = table["ai_bridge"]["switch_gate_best_move_max"].value_or(0);
        ai_bridge.switch_gate_require_negative_record = table["ai_bridge"]["switch_gate_require_negative_record"].value_or(true);
        ai_bridge.switch_gate_negative_record_threshold = table["ai_bridge"]["switch_gate_negative_record_threshold"].value_or(-20);
        ai_bridge.switch_gate_min_move_records = table["ai_bridge"]["switch_gate_min_move_records"].value_or(4);
        ai_bridge.score_survey_mode = table["ai_bridge"]["score_survey_mode"].value_or(false);
        ai_bridge.score_survey_dump_state = table["ai_bridge"]["score_survey_dump_state"].value_or(false);
        ai_bridge.score_survey_dump_readback_x0 = table["ai_bridge"]["score_survey_dump_readback_x0"].value_or(false);
        ai_bridge.score_survey_dump_readback_out = table["ai_bridge"]["score_survey_dump_readback_out"].value_or(false);
        ai_bridge.score_survey_words = table["ai_bridge"]["score_survey_words"].value_or(32);
        ai_bridge.action_probe_enabled = table["ai_bridge"]["action_probe_enabled"].value_or(false);
        ai_bridge.action_probe_action_id = table["ai_bridge"]["action_probe_action_id"].value_or(2);
        ai_bridge.action_probe_score = table["ai_bridge"]["action_probe_score"].value_or(1000000);
        ai_bridge.action_probe_max_uses = table["ai_bridge"]["action_probe_max_uses"].value_or(1);

        initialized = true;
    }
};

extern PatchConfig global_config;