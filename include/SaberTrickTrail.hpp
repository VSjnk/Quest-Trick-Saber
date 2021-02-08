#pragma once

#include "custom-types/shared/types.hpp"
#include "custom-types/shared/macros.hpp"
#include "GlobalNamespace/SaberTrail.hpp"

#include "UnityEngine/Transform.hpp"
#include "UnityEngine/Vector3.hpp"

#include "UnityEngine/Material.hpp"
#include "UnityEngine/MeshRenderer.hpp"
#include "UnityEngine/Color.hpp"
#include "GlobalNamespace/SaberMovementData.hpp"
#include "GlobalNamespace/IBladeMovementData.hpp"
#include "GlobalNamespace/BladeMovementDataElement.hpp"


DECLARE_CLASS_CODEGEN(TrickSaber, TrickSaberTrailData, GlobalNamespace::SaberTrail,

    DECLARE_INSTANCE_FIELD_DEFAULT(UnityEngine::Transform*, topTransform, nullptr);
    DECLARE_INSTANCE_FIELD_DEFAULT(UnityEngine::Transform*, bottomTransform, nullptr);
    DECLARE_INSTANCE_FIELD_DEFAULT(GlobalNamespace::SaberMovementData*, customMovementData, nullptr);
    DECLARE_INSTANCE_FIELD_DEFAULT(GlobalNamespace::SaberTrail*, saberTrail, nullptr);

    DECLARE_METHOD(void, Update);
    DECLARE_METHOD(void, print);
    DECLARE_METHOD(void, Awake);
    DECLARE_METHOD(void, Init, UnityEngine::Transform* topTransform, UnityEngine::Transform* bottomTransform, GlobalNamespace::SaberTrail* trail);


    REGISTER_FUNCTION(GlobalNamespace::SaberTrickTrail,
        REGISTER_METHOD(Update);
        REGISTER_METHOD(Awake);
        REGISTER_METHOD(Init);
        REGISTER_METHOD(print);

        REGISTER_FIELD(topTransform);
        REGISTER_FIELD(bottomTransform);
        REGISTER_FIELD(saberTrail);
        REGISTER_FIELD(customMovementData);

    )
)