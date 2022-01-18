#pragma once

#include "questui_components/shared/components/settings/StringSetting.hpp"

//(new StringSetting("",""))->with([](StringSetting* set){ \
//        set->mutateData([](MutableStringSettingsData data) { \
//            data.interactable = false; \
//            return data; \
//        }); \
//    })

namespace TrickSaberUI {
    auto SeparatorLine() {
        return QUC::StringSetting("", nullptr, "", true, false);
    }
}