#include <UnityEngine/MeshFilter.hpp>
#include "GlobalNamespace/TimeHelper.hpp"
#include "UnityEngine/Mathf.hpp"
#include "GlobalNamespace/TrailElementCollection.hpp"
#include "GlobalNamespace/TrailElement.hpp"
#include "GlobalNamespace/ColorManager.hpp"
#include "GlobalNamespace/SaberType.hpp"
#include "GlobalNamespace/SaberTrailRenderer.hpp"
#include "UnityEngine/GameObject.hpp"
#include "SaberTrickTrail.hpp"
#include "main.hpp"

DEFINE_CLASS(TrickSaber::TrickSaberTrailData);


namespace TrickSaber {


    void TrickSaberTrailData::Awake() {
        getLogger().info("Trick Data Awake");
        this->customMovementData = GlobalNamespace::SaberMovementData::New_ctor();

        this->saberTrail = get_gameObject()->GetComponent<GlobalNamespace::SaberTrail*>();
        this->saberTrail->movementData = reinterpret_cast<GlobalNamespace::IBladeMovementData*>(this->customMovementData);

        if (!this->topTransform){
            this->topTransform = this->get_transform()->Find(il2cpp_utils::createcsstr("TrailEnd"));
        }
        if (!this->bottomTransform){
            this->bottomTransform = this->get_transform()->Find(il2cpp_utils::createcsstr("TrailStart"));
        }
        set_enabled(true);
    }

    void TrickSaberTrailData::Update() {
        if (!this->topTransform || !this->bottomTransform)
        {
            if (this->topTransform == nullptr)
            {
                this->topTransform = this->get_transform()->Find(il2cpp_utils::createcsstr("TrailEnd"));
            }
            if (this->bottomTransform == nullptr)
            {
                this->bottomTransform = this->get_transform()->Find(il2cpp_utils::createcsstr("TrailStart"));
            }
            if (!this->topTransform || !this->bottomTransform) {
                getLogger().debug("No trails transform sadly");
                return;
            } else {
                getLogger().debug("YAY trails transform sadly");
            }
        }
        // this method just makes sure that the trail gets updated positions through it's custom movementData
        UnityEngine::Vector3 topPos = this->topTransform->get_position();
        UnityEngine::Vector3 bottomPos = this->bottomTransform->get_position();
        if (!this->customMovementData)
        {
            this->customMovementData = GlobalNamespace::SaberMovementData::New_ctor();
            this->saberTrail->movementData = reinterpret_cast<GlobalNamespace::IBladeMovementData*>(this->customMovementData);
        }

//        getLogger().debug("Trick trail add data");
        this->customMovementData->AddNewData(topPos, bottomPos, GlobalNamespace::TimeHelper::get_time());
    }
}
