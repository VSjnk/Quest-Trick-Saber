#pragma once

#include "main.hpp"
#include "ConfigEnums.hpp"

#include <string>
#include <unordered_map>

//#include "beatsaber-hook/shared/config/config-utils.hpp"
#include "config-utils/shared/config-utils.hpp"

DECLARE_CONFIG(PluginConfig,

DECLARE_VALUE(TriggerAction, int, "TriggerAction", (int) TrickAction::Throw);
DECLARE_VALUE(GripAction, int, "GripAction", (int) TrickAction::None);
DECLARE_VALUE(ThumbstickAction, int, "ThumbstickAction", (int)TrickAction::Spin);
DECLARE_VALUE(ButtonOneAction, int, "ButtonOneAction",(int) TrickAction::None);
DECLARE_VALUE(ButtonTwoAction, int, "ButtonTwoAction", (int) TrickAction::None);

DECLARE_VALUE(VibrateOnReturn, bool, "VibrateOnReturn", true);
DECLARE_VALUE(NoTricksWhileNotes, bool, "NoTricksWhileNotes", false);
DECLARE_VALUE(ReverseTrigger, bool, "ReverseTrigger", false);
DECLARE_VALUE(ReverseGrip, bool, "ReverseGrip", false);
DECLARE_VALUE(ReverseThumbstick, bool, "ReverseThumbstick", false);
DECLARE_VALUE(ReverseButtonOne, bool, "ReverseButtonOne", false);
DECLARE_VALUE(ReverseButtonTwo, bool, "ReverseButtonTwo", false);
DECLARE_VALUE(ThumbstickDirection, int, "ThumbstickDirection",(int)  ThumbstickDir::Horizontal);
DECLARE_VALUE(TriggerThreshold, float, "TriggerThreshold", 0.8f);
DECLARE_VALUE(GripThreshold, float, "GripThreshold", 0.8f);
DECLARE_VALUE(ThumbstickThreshold, float, "ThumbstickThreshold", 0.3f);
DECLARE_VALUE(ControllerSnapThreshold, float, "ControllerSnapThreshold", 0.3f);

DECLARE_VALUE(IsVelocityDependent, bool, "IsSpinVelocityDependent", false);

DECLARE_VALUE(SpinSpeed, float, "SpinSpeed", 1.0f);

DECLARE_VALUE(SpinDirection, int, "SpinDirection",(int)  SpinDir::Backward);

DECLARE_VALUE(ThrowVelocity, float, "ThrowVelocity", 1.0f);

DECLARE_VALUE(EnableTrickCutting, bool, "EnableTrickCutting (Broken)", false);
DECLARE_VALUE(CompleteRotationMode, bool, "CompleteRotationMode", false);

DECLARE_VALUE(ReturnSpeed, float, "ReturnSpeed", 10.0f);

DECLARE_VALUE(SlowmoDuringThrow, bool, "SlowmoDuringThrow", false);

DECLARE_VALUE(SlowmoAmount, float, "SlowmoAmount", 0.2f);

DECLARE_VALUE(VelocityBufferSize, int, "VelocityBufferSize", 5);

DECLARE_VALUE(SlowmoStepAmount, float, "SlowmoStepAmount", 0.02f);



    INIT_FUNCTION(
        INIT_VALUE(TriggerAction);

        INIT_VALUE(VibrateOnReturn);
        INIT_VALUE(GripAction);
        INIT_VALUE(ThumbstickAction);
        INIT_VALUE(ButtonOneAction);
        INIT_VALUE(ButtonTwoAction);
        INIT_VALUE(ReverseTrigger);
        INIT_VALUE(ReverseTrigger);
        INIT_VALUE(ReverseGrip);
        INIT_VALUE(ReverseThumbstick);
        INIT_VALUE(ReverseButtonOne);
        INIT_VALUE(ReverseButtonTwo);
        INIT_VALUE(ThumbstickDirection);
        INIT_VALUE(TriggerThreshold);
        INIT_VALUE(GripThreshold);
        INIT_VALUE(ThumbstickThreshold);
        INIT_VALUE(IsVelocityDependent);
        INIT_VALUE(SpinSpeed);
        INIT_VALUE(SpinDirection);
        INIT_VALUE(ControllerSnapThreshold);


        INIT_VALUE(ThrowVelocity);
        INIT_VALUE(EnableTrickCutting);
        INIT_VALUE(CompleteRotationMode);
        INIT_VALUE(ReturnSpeed);
        INIT_VALUE(SlowmoDuringThrow);
        INIT_VALUE(SlowmoAmount);
        INIT_VALUE(VelocityBufferSize);
        INIT_VALUE(SlowmoStepAmount);



    )
)
