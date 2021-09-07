#pragma once

#include <GlobalNamespace/OVRInput_Button.hpp>
#include "InputHandler.hpp"
#include "PluginConfig.hpp"
#include "AllEnums.hpp"

class ButtonHandler : public InputHandler {
  private:
    const GlobalNamespace::OVRInput::Controller _oculusController;
    const GlobalNamespace::OVRInput::Button _button;

  public:
    ButtonHandler(GlobalNamespace::OVRInput::Controller oculusController, GlobalNamespace::OVRInput::Button button)
      : InputHandler(0.5f), _oculusController(oculusController), _button(button)
    {
        IsReversed = button == GlobalNamespace::OVRInput::Button::One ? getPluginConfig().ReverseButtonOne.GetValue()
          : getPluginConfig().ReverseButtonTwo.GetValue();
    }

    float GetInputValue() override {
        return GlobalNamespace::OVRInput::Get(_button, _oculusController) ? 1.0f : 0.0f;
    }
};
