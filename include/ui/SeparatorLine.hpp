#pragma once

#include "questui_components/shared/components/settings/StringSetting.hpp"

//(new StringSetting("",""))->with([](StringSetting* set){ \
//        set->mutateData([](MutableStringSettingsData data) { \
//            data.interactable = false; \
//            return data; \
//        }); \
//    })

namespace TrickSaberUI {
    class SeparatorLine : public QuestUI_Components::StringSetting {
    public:
        explicit SeparatorLine() : StringSetting("","") {
            data.interactable = false;
        }
    };
}