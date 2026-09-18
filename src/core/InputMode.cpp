#include "core/InputMode.hpp"

#include "core/socd.hpp"
#include "core/state.hpp"
#include "util/state_util.hpp"

InputMode::InputMode() {}

GameModeConfig *InputMode::GetConfig() {
    return _config;
}

void InputMode::SetConfig(GameModeConfig &config) {
    _config = &config;
}

void InputMode::HandleSocd(InputState &inputs) {
    if (_config == nullptr) {
        return;
    }
    // Handle SOCD resolution for each SOCD button pair.
    for (size_t i = 0; i < _config->socd_pairs_count; i++) {
        const SocdPair &pair = _config->socd_pairs[i];
        switch (pair.socd_type) {
            case SOCD_NEUTRAL:
                socd::neutral(inputs, pair.button_dir1, pair.button_dir2);
                break;
            case SOCD_2IP:
                socd::second_input_priority(
                    inputs,
                    pair.button_dir1,
                    pair.button_dir2,
                    _socd_states[i]
                );
                break;
            case SOCD_2IP_NO_REAC:
                socd::second_input_priority_no_reactivation(
                    inputs,
                    pair.button_dir1,
                    pair.button_dir2,
                    _socd_states[i]
                );
                break;
            case SOCD_DIR1_PRIORITY:
                socd::dir1_priority(inputs, pair.button_dir1, pair.button_dir2);
                break;
            case SOCD_DIR2_PRIORITY:
                socd::dir1_priority(inputs, pair.button_dir2, pair.button_dir1);
                break;
            case SOCD_UNSPECIFIED:
            default:
                break;
        }
    }
}

void InputMode::HandleRemap(const InputState &original_inputs, InputState &remapped_inputs) {
    if (_config == nullptr) {
        return;
    }
    remapped_inputs.buttons = 0;

    // Track physical buttons that have been remapped so they are not left active as their
    // original input when they are intentionally mapped to a different action or macro chain.
    uint64_t physical_buttons_already_remapped = 0;
    for (size_t i = 0; i < _config->button_remapping_count; i++) {
        const ButtonRemap &remapping = _config->button_remapping[i];

        // Allow multiple remaps from the same physical button so intentional macro-like behavior
        // can be created via chained remapping. The physical button remains remapped away from
        // its original action, but each mapped target can still be activated.
        bool should_be_pressed = get_button(original_inputs.buttons, remapping.physical_button) ||
                                 get_button(remapped_inputs.buttons, remapping.activates);
        set_button(remapped_inputs.buttons, remapping.activates, should_be_pressed);

        // Track which physical buttons were remapped and remove their raw original state.
        set_button(physical_buttons_already_remapped, remapping.physical_button, true);
    }

    // Copy over original button states for buttons that were not remapped.
    remapped_inputs.buttons |= original_inputs.buttons & ~physical_buttons_already_remapped;
}
