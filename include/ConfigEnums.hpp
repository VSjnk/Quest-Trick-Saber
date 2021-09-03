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


DROPDOWN_CREATE_ENUM_CLASS(SpinDir,
                           STR_LIST("Forward", "Backward"),
                           Forward,
                           Backward)

DROPDOWN_CREATE_ENUM_CLASS(ThumbstickDir,
                           STR_LIST("Horizontal", "Vertical"),
                           Horizontal,
                           Vertical)