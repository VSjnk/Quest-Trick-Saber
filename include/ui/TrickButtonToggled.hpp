#pragma once

#include "UnityEngine/Transform.hpp"
#include "UnityEngine/RectTransform.hpp"
#include "UnityEngine/UI/LayoutElement.hpp"

#include "questui_components/shared/components/settings/ToggleSetting.hpp"

namespace TrickSaberUI {
    template<bool FixPos>
    class TrickButtonToggle : public QuestUI_Components::ConfigUtilsToggleSetting {
    public:
        template<typename... TArgs>
        explicit TrickButtonToggle(TArgs&&... args) : QuestUI_Components::ConfigUtilsToggleSetting(args...) {}

    protected:
        virtual Component *render(UnityEngine::Transform *parentTransform) override {
            auto comp = ConfigUtilsSetting::render(parentTransform);

            if constexpr(FixPos) {
                // fun fact, QuestUI gives me the transform of the toggle and BSML gives the transform of the parent of toggle & text.
                UnityEngine::RectTransform* rectTransform = getTransform()->get_parent()->template GetComponent<UnityEngine::RectTransform*>();

                CRASH_UNLESS(rectTransform);
                rectTransform->set_anchoredPosition({26, -15});
                rectTransform->set_sizeDelta({-130, 7});


                // if not parent
//                rectTransform->set_sizeDelta({110, 7});
//                getTransform()->get_parent()->set_localPosition(getTransform()->get_parent()->get_localPosition() + UnityEngine::Vector3{0,10,0});


// old test
//                rectTransform->set_anchoredPosition({0, 0});
//                rectTransform->set_sizeDelta({250, 7});

//                rectTransform->set_localPosition({0, 20, 0});
//                getTransform()->set_localPosition({0, 20, 0});

            }

            return comp;
        }

    public:
        void doUpdate() override {
            UpdateableComponent::doUpdate();
        }
    };
}