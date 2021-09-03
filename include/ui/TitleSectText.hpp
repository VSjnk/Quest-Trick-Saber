#pragma once

#include "questui_components/shared/components/Text.hpp"

namespace TrickSaberUI {
    static const auto sectTextMult = 6.0f;

    class TitleSectText : public QuestUI_Components::Text {
    public:
        explicit TitleSectText(std::string_view text, std::optional<InitialTextData> textData = std::nullopt) : Text(text, textData) {
            data.fontSize = sectTextMult;
        }
    };
}