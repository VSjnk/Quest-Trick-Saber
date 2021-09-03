#pragma once

#include "main.hpp"
#include "ConfigEnums.hpp"

#include <string>
#include <unordered_map>

//#include "beatsaber-hook/shared/config/config-utils.hpp"
#include "config-utils/shared/config-utils.hpp"

DECLARE_CONFIG(PluginConfig,

CONFIG_VALUE(TriggerAction, int, "TriggerAction", (int) TrickAction::Throw, "The action the trigger performs.");
CONFIG_VALUE(GripAction, int, "GripAction", (int) TrickAction::None, "The action the grip button performs.");
CONFIG_VALUE(ThumbstickAction, int, "ThumbstickAction", (int)TrickAction::Spin, "The action the thumbstick performs.");
CONFIG_VALUE(ButtonOneAction, int, "ButtonOneAction",(int) TrickAction::None, "The action the button one performs.");
CONFIG_VALUE(ButtonTwoAction, int, "ButtonTwoAction", (int) TrickAction::None, "The action the button two performs.");

CONFIG_VALUE(VibrateOnReturn, bool, "VibrateOnReturn", true, "Makes the controller vibrate when it returns from being thrown");
CONFIG_VALUE(NoTricksWhileNotes, bool, "NoTrMakes the controller vibrate when it returns from being thrownicksWhileNotes", false, "Doesn't allow tricks while notes are on screen");
CONFIG_VALUE(ReverseTrigger, bool, "ReverseTrigger", false, "Inverts the trigger button");
CONFIG_VALUE(ReverseGrip, bool, "ReverseGrip", false, "Inverts the grip toggle.");
CONFIG_VALUE(ReverseThumbstick, bool, "ReverseThumbstick", false, "Inverts the thumbstick direction.");
CONFIG_VALUE(ReverseButtonOne, bool, "ReverseButtonOne", false, "Inverts the button one toggle.");
CONFIG_VALUE(ReverseButtonTwo, bool, "ReverseButtonTwo", false, "Inverts the button two toggle.");
CONFIG_VALUE(ThumbstickDirection, int, "ThumbstickDirection",(int)  ThumbstickDir::Horizontal, "The direction of the thumbsticks for tricks.");
CONFIG_VALUE(TriggerThreshold, float, "TriggerThreshold", 0.8f, "The deadzone or minimum amount of input required to trigger.");
CONFIG_VALUE(GripThreshold, float, "GripThreshold", 0.8f, "The deadzone or minimum amount of input required to trigger the grip.");
CONFIG_VALUE(ThumbstickThreshold, float, "ThumbstickThreshold", 0.3f, "The deadzone or minimum amount of input required to trigger the thumbstick.");
CONFIG_VALUE(ControllerSnapThreshold, float, "ControllerSnapThreshold", 0.3f, "The deadzone or minimum amount of input required for the controller to snap.");

CONFIG_VALUE(IsVelocityDependent, bool, "IsSpinVelocityDependent", false, "Makes the spin speed velocity dependent.");

CONFIG_VALUE(SpinSpeed, float, "SpinSpeed", 1.0f, "The speed the saber spins at.");

CONFIG_VALUE(SpinDirection, int, "SpinDirection",(int)  SpinDir::Backward, "The direction of spinning. Still dependent on reverse button");

CONFIG_VALUE(ThrowVelocity, float, "ThrowVelocity", 1.0f, "The velocity of the saber when you throw it.");

CONFIG_VALUE(EnableTrickCutting, bool, "EnableTrickCutting (Broken)", false);
CONFIG_VALUE(CompleteRotationMode, bool, "CompleteRotationMode", false, "Allows for the spin rotation to go all directions.");
CONFIG_VALUE(MoveWhileThrown, bool, "Move While Thrown", false, "Makes the saber while thrown be physically affected by your controller rotation.");

CONFIG_VALUE(ReturnSpeed, float, "ReturnSpeed", 10.0f, "The speed in which the saber returns to your hand.");

CONFIG_VALUE(SlowmoDuringThrow, bool, "SlowmoDuringThrow", false, "Makes the thrown saber act slow-mo like.");

CONFIG_VALUE(SlowmoAmount, float, "SlowmoAmount", 0.2f, "The intensity of the slow motion.");

CONFIG_VALUE(VelocityBufferSize, float, "VelocityBufferSize", 5, "Technical number for the size of the list that holds velocity throughout time.");

CONFIG_VALUE(SlowmoStepAmount, float, "SlowmoStepAmount", 0.02f, "The slow motion time scale amount.");



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
        CONFIG_INIT_VALUE(MoveWhileThrown);
        CONFIG_INIT_VALUE(ReturnSpeed);
        CONFIG_INIT_VALUE(SlowmoDuringThrow);
        CONFIG_INIT_VALUE(SlowmoAmount);
        CONFIG_INIT_VALUE(VelocityBufferSize);
        CONFIG_INIT_VALUE(SlowmoStepAmount);



    )
)
