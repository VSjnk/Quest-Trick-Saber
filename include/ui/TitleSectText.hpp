#pragma once

#include "questui_components/shared/components/Text.hpp"

namespace TrickSaberUI {
    static const auto SECT_TEXT_MULT = 6.0f;

    // make a class, so we can reorder arguments in a less messy way
    class TitleSectText : public QUC::Text {
    public:
        TitleSectText(std::string_view const &t, float fontSize = SECT_TEXT_MULT)
                : Text(t, true, std::nullopt, fontSize) {}
    };
}