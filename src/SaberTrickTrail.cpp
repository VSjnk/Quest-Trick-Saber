#include <UnityEngine/MeshFilter.hpp>
#include "GlobalNamespace/TimeHelper.hpp"
#include "UnityEngine/Mathf.hpp"
#include "GlobalNamespace/TrailElementCollection.hpp"
#include "GlobalNamespace/TrailElement.hpp"
#include "GlobalNamespace/SaberTrailRenderer.hpp"
#include "UnityEngine/GameObject.hpp"
#include "SaberTrickTrail.hpp"
#include "main.hpp"

DEFINE_TYPE(TrickSaber::TrickSaberTrailData);


namespace TrickSaber {

    void TrickSaberTrailData::Init(UnityEngine::Transform* topTransform, UnityEngine::Transform* bottomTransform, GlobalNamespace::SaberTrail* saberTrail1) {
        this->topTransform = topTransform;
        this->bottomTransform = bottomTransform;
        this->trailDuration = saberTrail1->trailDuration;
        this->whiteSectionMaxDuration = saberTrail1->whiteSectionMaxDuration;
        this->trailRenderer = saberTrail1->trailRenderer;
        this->trailRendererPrefab = saberTrail1->trailRendererPrefab;
        this->color = saberTrail1->color;
        this->granularity = saberTrail1->granularity;
        set_enabled(saberTrail1->get_enabled());
    }

    void TrickSaberTrailData::Update() {
        if (!this->topTransform || !this->bottomTransform) {
            if (this->topTransform == nullptr) {
                this->topTransform = this->get_transform()->Find(il2cpp_utils::createcsstr("TrailEnd"));
            }
            if (this->bottomTransform == nullptr) {
                this->bottomTransform = this->get_transform()->Find(il2cpp_utils::createcsstr("TrailStart"));
            }
            if (!this->topTransform || !this->bottomTransform) {
                return;
            }
        }
        // this method just makes sure that the trail gets updated positions through it's custom movementData
        UnityEngine::Vector3 topPos = this->topTransform->get_position();
        UnityEngine::Vector3 bottomPos = this->bottomTransform->get_position();
        if (!this->customMovementData) {
            this->customMovementData = GlobalNamespace::SaberMovementData::New_ctor();
        }

        this->movementData = reinterpret_cast<GlobalNamespace::IBladeMovementData *>(this->customMovementData);

//        getLogger().debug("Trick trail add data saber: %s (%d %d %d) (%d %d %d)",
//                          to_utf8(csstrtostr(get_gameObject()->get_name())).c_str(),
//                          topPos.x, topPos.y, topPos.z,
//                          bottomPos.x, bottomPos.y, bottomPos.z
//        );

        this->customMovementData->AddNewData(topPos, bottomPos, GlobalNamespace::TimeHelper::get_time());
    }
}
