#pragma once

#include "InputHandler.hpp"
#include "PluginConfig.hpp"
#include "AllEnums.hpp"
#include "UnityEngine/Input.hpp"

class TriggerHandler : public InputHandler {
  private:
    Il2CppString* _inputString;

  public:
    TriggerHandler(UnityEngine::XR::XRNode node, float threshold) : InputHandler(threshold) {
        // axis names are from HMLib's VRControllersInputManager
        auto str = (node == UnityEngine::XR::XRNode::LeftHand) ? "TriggerLeftHand" : "TriggerRightHand";
        _inputString = il2cpp_utils::newcsstr(str);
        IsReversed = getPluginConfig().ReverseTrigger.GetValue();
    }

    float GetInputValue() override {
        // if (val != 0) getLogger().debug("TriggerHandler input value: %f", val);
        return UnityEngine::Input::GetAxis(_inputString);
    }
};