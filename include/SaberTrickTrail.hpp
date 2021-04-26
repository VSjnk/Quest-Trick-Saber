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

    DECLARE_INSTANCE_FIELD(UnityEngine::Transform*, topTransform);
    DECLARE_INSTANCE_FIELD(UnityEngine::Transform*, bottomTransform);
    DECLARE_INSTANCE_FIELD(GlobalNamespace::SaberMovementData*, customMovementData);
    DECLARE_METHOD(void, Update);
    DECLARE_METHOD(void, Init, UnityEngine::Transform* topTransform, UnityEngine::Transform* bottomTransform, GlobalNamespace::SaberTrail* trail);


    REGISTER_FUNCTION(
        REGISTER_METHOD(Update);
        REGISTER_METHOD(Init);

        REGISTER_FIELD(topTransform);
        REGISTER_FIELD(bottomTransform);
        REGISTER_FIELD(customMovementData);
    )
)