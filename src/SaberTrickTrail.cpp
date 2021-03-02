#include <UnityEngine/MeshFilter.hpp>
#include "GlobalNamespace/TimeHelper.hpp"
#include "UnityEngine/Mathf.hpp"
#include "GlobalNamespace/TrailElementCollection.hpp"
#include "GlobalNamespace/TrailElement.hpp"
#include "GlobalNamespace/SaberTrailRenderer.hpp"
#include "UnityEngine/GameObject.hpp"
#include "SaberTrickTrail.hpp"
#include "main.hpp"

DEFINE_CLASS(TrickSaber::TrickSaberTrailData);


namespace TrickSaber {


    void TrickSaberTrailData::Awake() {
        getLogger().info("Trick Data Awake");
        this->customMovementData = GlobalNamespace::SaberMovementData::New_ctor();

        this->movementData = reinterpret_cast<GlobalNamespace::IBladeMovementData*>(this->customMovementData);

        if (!this->topTransform){
            this->topTransform = this->get_transform()->Find(il2cpp_utils::createcsstr("TrailEnd"));
        }
        if (!this->bottomTransform){
            this->bottomTransform = this->get_transform()->Find(il2cpp_utils::createcsstr("TrailStart"));
        }

        print();

        set_enabled(true);
    }

    void TrickSaberTrailData::Init(UnityEngine::Transform* topTransform, UnityEngine::Transform* bottomTransform, GlobalNamespace::SaberTrail* saberTrail1) {
        this->topTransform = topTransform;
        this->bottomTransform = bottomTransform;
        this->trailDuration = saberTrail1->trailDuration;
        this->whiteSectionMaxDuration = saberTrail1->whiteSectionMaxDuration;
        this->trailRenderer = saberTrail1->trailRenderer;
        this->trailRendererPrefab = saberTrail1->trailRendererPrefab;
        this->color = saberTrail1->color;
        this->granularity = saberTrail1->granularity;
        this->saberTrail = saberTrail1;

        print();
    }

    void TrickSaberTrailData::Update() {
        this->trailDuration = saberTrail->trailDuration;
        this->whiteSectionMaxDuration = saberTrail->whiteSectionMaxDuration;
        this->trailRenderer = saberTrail->trailRenderer;
        this->trailRendererPrefab = saberTrail->trailRendererPrefab;
        this->color = saberTrail->color;
        this->granularity = saberTrail->granularity;

        if (!this->topTransform || !this->bottomTransform) {
            if (this->topTransform == nullptr) {
                this->topTransform = this->get_transform()->Find(il2cpp_utils::createcsstr("TrailEnd"));
            }
            if (this->bottomTransform == nullptr) {
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

    void TrickSaberTrailData::print() {
        if (topTransform->get_gameObject() && bottomTransform->get_gameObject()) {
            auto *topGOName = UnityEngine::GameObject::GetName(topTransform->get_gameObject());
            auto *botGOName = UnityEngine::GameObject::GetName(bottomTransform->get_gameObject());

            auto topName = to_utf8(csstrtostr(topGOName));
            auto botName = to_utf8(csstrtostr(botGOName));

            if (topGOName && botGOName) {
                getLogger().debug("Saber transforms redo top %s bottom %s",
                                  topName.c_str(),
                                  botName.c_str()
                );
            }
        }
    }
}
