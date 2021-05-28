#pragma once

#include "InputHandler.hpp"
#include "PluginConfig.hpp"
#include "AllEnums.hpp"

#include "UnityEngine/XR/XRNode.hpp"
#include <string>

class ThumbstickHandler : public InputHandler {
  private:
    Il2CppString* _inputString;

  public:
    ThumbstickHandler(UnityEngine::XR::XRNode node, float threshold, int thumstickDir) : InputHandler(threshold) {
        // axis names are from HMLib's VRControllersInputManager
        std::string axis = thumstickDir == (int) ThumbstickDir::Horizontal ? "Horizontal" : "Vertical";
        axis += node == UnityEngine::XR::XRNode::LeftHand ? "LeftHand" : "RightHand";
        _inputString = il2cpp_utils::newcsstr(axis);
        IsReversed = getPluginConfig().ReverseThumbstick.GetValue();
    }

    float GetInputValue() {
        // if (val != 0) getLogger().debug("ThumbstickHandler input value: %f", val);
        return UnityEngine::Input::GetAxis(_inputString);
    }
};