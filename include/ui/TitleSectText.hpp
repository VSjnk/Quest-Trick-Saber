#pragma once

#include "questui_components/shared/components/Text.hpp"

namespace TrickSaberUI {
    static const auto SECT_TEXT_MULT = 6.0f;

    // make a class, so we can reorder arguments in a less messy way
    class TitleSectText : public QUC::Text {
    public:
        TitleSectText(std::string_view const &t, float fontSize = SECT_TEXT_MULT, bool enabled = true,
                      std::optional<Sombrero::FastColor> const &c = std::nullopt,
                      bool italic_ = true, UnityEngine::Vector2 anch = {0.0f, 0.0f},
                      UnityEngine::Vector2 sd = {60.0f, 10.0f})
                : Text(t, enabled, c, fontSize, italic_, anch, sd) {}
    };
}