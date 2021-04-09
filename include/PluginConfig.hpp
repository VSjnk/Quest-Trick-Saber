#pragma once

#include "main.hpp"
#include "ConfigEnums.hpp"

#include <string>
#include <unordered_map>

//#include "beatsaber-hook/shared/config/config-utils.hpp"
#include "config-utils/shared/config-utils.hpp"

DECLARE_CONFIG(PluginConfig,

CONFIG_VALUE(TriggerAction, int, "TriggerAction", (int) TrickAction::Throw);
CONFIG_VALUE(GripAction, int, "GripAction", (int) TrickAction::None);
CONFIG_VALUE(ThumbstickAction, int, "ThumbstickAction", (int)TrickAction::Spin);
CONFIG_VALUE(ButtonOneAction, int, "ButtonOneAction",(int) TrickAction::None);
CONFIG_VALUE(ButtonTwoAction, int, "ButtonTwoAction", (int) TrickAction::None);

CONFIG_VALUE(VibrateOnReturn, bool, "VibrateOnReturn", true);
CONFIG_VALUE(NoTricksWhileNotes, bool, "NoTricksWhileNotes", false);
CONFIG_VALUE(ReverseTrigger, bool, "ReverseTrigger", false);
CONFIG_VALUE(ReverseGrip, bool, "ReverseGrip", false);
CONFIG_VALUE(ReverseThumbstick, bool, "ReverseThumbstick", false);
CONFIG_VALUE(ReverseButtonOne, bool, "ReverseButtonOne", false);
CONFIG_VALUE(ReverseButtonTwo, bool, "ReverseButtonTwo", false);
CONFIG_VALUE(ThumbstickDirection, int, "ThumbstickDirection",(int)  ThumbstickDir::Horizontal);
CONFIG_VALUE(TriggerThreshold, float, "TriggerThreshold", 0.8f);
CONFIG_VALUE(GripThreshold, float, "GripThreshold", 0.8f);
CONFIG_VALUE(ThumbstickThreshold, float, "ThumbstickThreshold", 0.3f);
CONFIG_VALUE(ControllerSnapThreshold, float, "ControllerSnapThreshold", 0.3f);

CONFIG_VALUE(IsVelocityDependent, bool, "IsSpinVelocityDependent", false);

CONFIG_VALUE(SpinSpeed, float, "SpinSpeed", 1.0f);

CONFIG_VALUE(SpinDirection, int, "SpinDirection",(int)  SpinDir::Backward);

CONFIG_VALUE(ThrowVelocity, float, "ThrowVelocity", 1.0f);

CONFIG_VALUE(EnableTrickCutting, bool, "EnableTrickCutting (Broken)", false);
CONFIG_VALUE(CompleteRotationMode, bool, "CompleteRotationMode", false);

CONFIG_VALUE(ReturnSpeed, float, "ReturnSpeed", 10.0f);

CONFIG_VALUE(SlowmoDuringThrow, bool, "SlowmoDuringThrow", false);

CONFIG_VALUE(SlowmoAmount, float, "SlowmoAmount", 0.2f);

CONFIG_VALUE(VelocityBufferSize, int, "VelocityBufferSize", 5);

CONFIG_VALUE(SlowmoStepAmount, float, "SlowmoStepAmount", 0.02f);



    CONFIG_INIT_FUNCTION(
        CONFIG_INIT_VALUE(TriggerAction);

        CONFIG_INIT_VALUE(VibrateOnReturn);
        CONFIG_INIT_VALUE(NoTricksWhileNotes);
        CONFIG_INIT_VALUE(GripAction);
        CONFIG_INIT_VALUE(ThumbstickAction);
        CONFIG_INIT_VALUE(ButtonOneAction);
        CONFIG_INIT_VALUE(ButtonTwoAction);
        CONFIG_INIT_VALUE(ReverseTrigger);
        CONFIG_INIT_VALUE(ReverseTrigger);
        CONFIG_INIT_VALUE(ReverseGrip);
        CONFIG_INIT_VALUE(ReverseThumbstick);
        CONFIG_INIT_VALUE(ReverseButtonOne);
        CONFIG_INIT_VALUE(ReverseButtonTwo);
        CONFIG_INIT_VALUE(ThumbstickDirection);
        CONFIG_INIT_VALUE(TriggerThreshold);
        CONFIG_INIT_VALUE(GripThreshold);
        CONFIG_INIT_VALUE(ThumbstickThreshold);
        CONFIG_INIT_VALUE(IsVelocityDependent);
        CONFIG_INIT_VALUE(SpinSpeed);
        CONFIG_INIT_VALUE(SpinDirection);
        CONFIG_INIT_VALUE(ControllerSnapThreshold);


        CONFIG_INIT_VALUE(ThrowVelocity);
        CONFIG_INIT_VALUE(EnableTrickCutting);
        CONFIG_INIT_VALUE(CompleteRotationMode);
        CONFIG_INIT_VALUE(ReturnSpeed);
        CONFIG_INIT_VALUE(SlowmoDuringThrow);
        CONFIG_INIT_VALUE(SlowmoAmount);
        CONFIG_INIT_VALUE(VelocityBufferSize);
        CONFIG_INIT_VALUE(SlowmoStepAmount);



    )
)
