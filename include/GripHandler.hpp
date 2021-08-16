#pragma once

#include <functional>
#include <UnityEngine/XR/InputDevice.hpp>
#include "GlobalNamespace/OVRInput_Axis1D.hpp"
#include "InputHandler.hpp"
#include "PluginConfig.hpp"
#include "AllEnums.hpp"

class GripHandler : public InputHandler {
  private:
    const GlobalNamespace::OVRInput::Controller _oculusController;
    UnityEngine::XR::InputDevice _controllerInputDevice;  // UnityEngine.XR.InputDevice
    bool connected = false;

    typedef float (GripHandler::*MemFn)();
    MemFn _valueFunc;

    float GetValueSteam() {
        // this class can't even be found on Quest:
        static auto* commonUsages = CRASH_UNLESS(il2cpp_utils::GetClassFromName("UnityEngine.XR", "CommonUsages"));
        auto* grip = CRASH_UNLESS(il2cpp_utils::GetFieldValue(commonUsages, "grip"));
        float outvar;
        // argument types are important for finding the correct match
        if (CRASH_UNLESS(il2cpp_utils::RunMethod<bool>(_controllerInputDevice, "TryGetFeatureValue", grip, outvar))) {
            return outvar;
        }
        return 0;
    }

    //CommomUsages doesn't work well with Touch Controllers, so we need to use the Oculus function for them
    float GetValueOculus() {
        static auto primaryHandTrigger = GlobalNamespace::OVRInput::Axis1D::PrimaryHandTrigger;
        // let il2cpp_utils cache the method
        return GlobalNamespace::OVRInput::Get(primaryHandTrigger, _oculusController);
    }

  public:
    GripHandler(bool isConnected, GlobalNamespace::OVRInput::Controller oculusController, UnityEngine::XR::InputDevice controllerInputDevice, float threshold)
    : InputHandler(threshold), _oculusController(oculusController), _controllerInputDevice(controllerInputDevice), connected(isConnected) {
        _valueFunc = &GripHandler::GetValueOculus;

        IsReversed = getPluginConfig().ReverseGrip.GetValue();
    }

    #define CALL_MEMFN_ON_PTR(ptr,memFn)  ((ptr)->*(memFn))

    float GetInputValue() {
        auto val = CALL_MEMFN_ON_PTR(this, _valueFunc)();
        // if (val != 0) getLogger().debug("GripHandler input value: %f", val);
        return val;
    }
};