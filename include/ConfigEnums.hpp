#pragma once

#include <initializer_list>
#include <string>
#include <unordered_map>
#include <map>

#include "questui_components/shared/components/settings/DropdownSetting.hpp"

DROPDOWN_CREATE_ENUM_CLASS(TrickAction,
                           STR_LIST("Throw",
                                    "Spin",
                                    "None",
                                    "FreezeThrow"),
                           Throw,
                           Spin,
                           None,
                           FreezeThrow)
static const size_t TRICK_ACTION_COUNT = 4;


DROPDOWN_CREATE_ENUM_CLASS(SpinDir,
                           STR_LIST("Forward", "Backward"),
                           Forward,
                           Backward)

static const size_t SPIN_DIR_COUNT = 2;

DROPDOWN_CREATE_ENUM_CLASS(ThumbstickDir,
                           STR_LIST("Horizontal", "Vertical"),
                           Horizontal,
                           Vertical)

static const size_t THUMBSTICK_DIR_COUNT = 4;