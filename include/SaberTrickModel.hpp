#pragma once

#include "PluginConfig.hpp"
#include "beatsaber-hook/shared/utils/il2cpp-functions.hpp"
#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"
#include "beatsaber-hook/shared/utils/utils.h"
#include "beatsaber-hook/shared/utils/typedefs-array.hpp"
#include "beatsaber-hook/shared/utils/typedefs.h"
#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/Rigidbody.hpp"
#include "UnityEngine/Transform.hpp"
#include "UnityEngine/Collider.hpp"
#include "UnityEngine/MeshFilter.hpp"
#include "UnityEngine/MeshRenderer.hpp"
#include "UnityEngine/MaterialPropertyBlock.hpp"
#include "GlobalNamespace/Saber.hpp"
#include "GlobalNamespace/SaberTrailRenderer.hpp"
#include "GlobalNamespace/SaberType.hpp"
#include "GlobalNamespace/SetSaberGlowColor.hpp"
#include "GlobalNamespace/SetSaberFakeGlowColor.hpp"
#include "GlobalNamespace/SaberTypeObject.hpp"
#include "GlobalNamespace/SaberModelContainer.hpp"
#include "GlobalNamespace/SaberModelController.hpp"
#include "GlobalNamespace/SaberTrail.hpp"
#include "GlobalNamespace/SaberMovementData.hpp"
#include "GlobalNamespace/TubeBloomPrePassLight.hpp"
#include "GlobalNamespace/SaberModelController_InitData.hpp"
#include "GlobalNamespace/SetSaberGlowColor_PropertyTintColorPair.hpp"
#include "GlobalNamespace/Parametric3SliceSpriteController.hpp"
#include "GlobalNamespace/SaberManager.hpp"
#include "GlobalNamespace/SaberBurnMarkSparkles.hpp"
#include "GlobalNamespace/ColorManager.hpp"

#include "System/Collections/Generic/List_1.hpp"
#include "SaberTrickTrail.hpp"
#include <string>

static const std::string saberPrefix = "trick_saber_";

class SaberTrickModel {
  public:
    UnityEngine::Rigidbody* Rigidbody = nullptr;
    UnityEngine::GameObject* SaberGO;  // GameObject
    GlobalNamespace::ColorManager* colorManager;
    GlobalNamespace::Saber* saberScript;
    bool basicSaber;
    int update;

    SaberTrickModel(GlobalNamespace::Saber* saber, UnityEngine::GameObject* SaberModel, bool basicSaber) {
        CRASH_UNLESS(SaberModel);
        this->basicSaber = basicSaber;
        saberScript = saber;
        getLogger().debug("SaberTrickModel construction! %s", basicSaber ? "true" : "false");
        // il2cpp_utils::LogClass(il2cpp_functions::class_from_system_type(tRigidbody), false);

        SaberGO = OriginalSaberModel = SaberModel;

        if (getPluginConfig().EnableTrickCutting.GetValue()) {
            Rigidbody = SaberModel->GetComponent<UnityEngine::Rigidbody*>();
            if (!Rigidbody) {
                getLogger().warning("Adding rigidbody to original SaberModel?!");
                Rigidbody = SaberModel->AddComponent<UnityEngine::Rigidbody*>();
            }
            SetupRigidbody(Rigidbody, OriginalSaberModel);
        } else {
            TrickModel = UnityEngine::Object::Instantiate(SaberModel);
            CRASH_UNLESS(TrickModel);
            auto* newName = il2cpp_utils::createcsstr(
                    saberPrefix + to_utf8(csstrtostr(SaberModel->get_name())) + "_" + std::to_string((int) saber->saberType->saberType)
            );
            UnityEngine::Object::SetName(TrickModel, newName);
            getLogger().debug("Trick model name: %s", to_utf8(csstrtostr(TrickModel->get_name())).c_str());
            FixBasicTrickSaber(TrickModel, basicSaber);
            AddTrickRigidbody();



            auto* str = CRASH_UNLESS(il2cpp_utils::createcsstr("VRGameCore"));
            auto* vrGameCore = UnityEngine::GameObject::Find(str);
            auto* vrGameCoreT = vrGameCore->get_transform();
            auto* trickModelT = TrickModel->get_transform();

            trickModelT->SetParent(vrGameCoreT);
            TrickModel->SetActive(false);
            OriginalSaberModel->SetActive(true);
        }

        getLogger().debug("Leaving SaberTrickModel construction!");
    }

    void SetupRigidbody(UnityEngine::Rigidbody* rigidbody, UnityEngine::GameObject* model) {
        getLogger().debug("Rigid body pos: (%f %f %f) (%f %f %f)", rigidbody->get_position().x, rigidbody->get_position().y, rigidbody->get_position().z, rigidbody->get_transform()->get_localPosition().x, rigidbody->get_transform()->get_localPosition().y, rigidbody->get_transform()->get_localPosition().z);
        rigidbody->set_useGravity(false);
        rigidbody->set_isKinematic(true);

        static auto set_detectCollisions = (function_ptr_t<void, Il2CppObject*, bool>)CRASH_UNLESS(
            il2cpp_functions::resolve_icall("UnityEngine.Rigidbody::set_detectCollisions"));
        getLogger().debug("set_detectCollisions ptr offset: %lX", asOffset(set_detectCollisions));
        set_detectCollisions(rigidbody, false);

        static auto set_maxAngVel = (function_ptr_t<void, Il2CppObject*, float>)CRASH_UNLESS(
            il2cpp_functions::resolve_icall("UnityEngine.Rigidbody::set_maxAngularVelocity"));
        getLogger().debug("set_maxAngVel ptr offset: %lX", asOffset(set_maxAngVel));
        set_maxAngVel(rigidbody, 800.0f);

        static auto set_interp = (function_ptr_t<void, Il2CppObject*, int>)CRASH_UNLESS(
            il2cpp_functions::resolve_icall("UnityEngine.Rigidbody::set_interpolation"));
        getLogger().debug("set_interpolation ptr offset: %lX", asOffset(set_interp));
        set_interp(rigidbody, 1);  // Interpolate

        Array<UnityEngine::Collider*>* colliders = model->GetComponentsInChildren<UnityEngine::Collider*>(true); //CRASH_UNLESS(il2cpp_utils::RunMethod<Array<Il2CppObject*>*>(model, "GetComponentsInChildren", tCollider, true));


        for (int i = 0; i < colliders->Length(); i++) {
            colliders->values[i]->set_enabled(false);
        }
    }

    void AddTrickRigidbody() {
        Rigidbody = TrickModel->AddComponent<UnityEngine::Rigidbody*>(); //CRASH_UNLESS(il2cpp_utils::RunMethod(TrickModel, "AddComponent", tRigidbody));
        CRASH_UNLESS(Rigidbody);
        SetupRigidbody(Rigidbody, TrickModel);
    }

    void FixBasicTrickSaber(UnityEngine::GameObject* newSaber, bool basic) {
        if (!basic) return;
        getLogger().debug("Fixing basic trick saber color!");

        GlobalNamespace::SaberModelContainer* saberModelContainer = SaberGO->GetComponentsInParent<GlobalNamespace::SaberModelContainer*>(false)->values[0];

        GlobalNamespace::SaberTypeObject* _saberTypeObject = saberModelContainer->saber->saberType;
        GlobalNamespace::SaberType saberType = _saberTypeObject->saberType; // CRASH_UNLESS(il2cpp_utils::GetPropertyValue(_saberTypeObject, "saberType"));
        getLogger().debug("saber type: %i", (int) saberType);
//        CRASH_UNLESS(saberType);
        auto* saberModelContainerT = saberModelContainer->get_transform();
        getLogger().debug("saber container");
        CRASH_UNLESS(saberModelContainerT);

        auto* saberModelController = newSaber->GetComponent<GlobalNamespace::SaberModelController*>();
        getLogger().debug("saber controller");
        CRASH_UNLESS(saberModelController);

        auto* origModelController = SaberGO->GetComponent<GlobalNamespace::SaberModelController*>(); // CRASH_UNLESS(il2cpp_utils::RunMethod(SaberGO, "GetComponent", tSaberModelController));
        auto* colorMgr = origModelController->colorManager; // CRASH_UNLESS(il2cpp_utils::GetFieldValue(origModelController, "_colorManager"));
        getLogger().debug("saber color manager");
        CRASH_UNLESS(colorMgr);
        saberModelController->colorManager = colorMgr;

        auto* glows = saberModelController->setSaberGlowColors; // CRASH_UNLESS(il2cpp_utils::GetFieldValue<Il2CppArray*>(saberModelController, "_setSaberGlowColors"));
        getLogger().debug("_setSaberGlowColors.length: %u", glows->Length());
        for (int i = 0; i < glows->Length(); i++) {
            GlobalNamespace::SetSaberGlowColor* obj = glows->values[i];

            obj->colorManager = colorMgr;
        }

        auto* fakeGlows = saberModelController->setSaberFakeGlowColors;
        getLogger().debug("_setSaberFakeGlowColors.length: %u", fakeGlows->Length());
        for (int i = 0; i < fakeGlows->Length(); i++) {
            GlobalNamespace::SetSaberFakeGlowColor* obj = fakeGlows->values[i];

            obj->colorManager = colorMgr;

        }

        auto* trickT = TrickModel->get_transform(); // CRASH_UNLESS(il2cpp_utils::GetPropertyValue(TrickModel, "transform"));
        auto* origT = OriginalSaberModel->get_transform(); // CRASH_UNLESS(il2cpp_utils::GetPropertyValue(OriginalSaberModel, "transform"));

        auto pos = origT->get_position(); //CRASH_UNLESS(il2cpp_utils::GetPropertyValue<Vector3>(origT, "position"));
        auto rot = origT->get_rotation(); //CRASH_UNLESS(il2cpp_utils::GetPropertyValue<Quaternion>(origT, "rotation"));

        trickT->set_position(pos);
        trickT->set_rotation(rot);


        saberModelController->Init(saberModelContainerT, saberModelContainer->saber);

        if (basicSaber && !Modloader::getMods().contains("Qosmetics")) {
//                TrickModel->GetComponent<GlobalNamespace::SaberTrail*>()->movementData = reinterpret_cast<GlobalNamespace::IBladeMovementData*>(GlobalNamespace::SaberMovementData::New_ctor());
            // Fix trails?
            TrickModel->SetActive(true);
            FixTrails(TrickModel);
            FixTrails(OriginalSaberModel);
            TrickModel->SetActive(false);
        }
    }

    void FixTrails(UnityEngine::GameObject* model) {
        auto* trails = model->GetComponentsInChildren<GlobalNamespace::SaberTrail*>(false);
        getLogger().debug("trick saber trails.length: %u", trails->Length());
        for (int i = 0; i < trails->Length(); i++) {
            GlobalNamespace::SaberTrail *obj = trails->values[i];

            auto *trailStart = UnityEngine::GameObject::New_ctor();
            auto *trailEnd = UnityEngine::GameObject::New_ctor();


//                trailStart->get_transform()->SetParent(saberScript->saberBladeBottomTransform, false);
            trailStart->get_transform()->SetPositionAndRotation(saberScript->saberBladeBottomTransform->get_position(), saberScript->saberBladeBottomTransform->get_rotation());
            trailStart->get_transform()->set_position(saberScript->saberBladeBottomTransform->get_position());
            trailStart->get_transform()->SetParent(model->get_transform());
            //                trailStart->get_transform()->set_localPosition(saberScript->saberBladeBottomTransform->get_position());
//                trailStart->get_transform()->set_localRotation(saberScript->saberBladeBottomTransform->get_rotation());
//                trailStart->get_transform()->set_localPosition(UnityEngine::Vector3(0.0, 0.0, 1.02));


//                trailEnd->get_transform()->SetParent(saberScript->saberBladeTopTransform, false);
            trailEnd->get_transform()->SetPositionAndRotation(saberScript->saberBladeTopTransform->get_position(), saberScript->saberBladeTopTransform->get_rotation());
            trailEnd->get_transform()->set_position(saberScript->saberBladeTopTransform->get_position());
            trailEnd->get_transform()->SetParent(model->get_transform());
            getLogger().debug("Trail end parent: %s", to_utf8(csstrtostr(trailEnd->get_transform()->GetParent()->get_name())).c_str());

            //                trailEnd->get_transform()->set_localPosition(UnityEngine::Vector3(0.0, 0.0, 0.1));
//                trailEnd->get_transform()->set_localPosition(saberScript->saberBladeTopTransform->get_position());
//                trailEnd->get_transform()->set_localRotation(saberScript->saberBladeTopTransform->get_rotation());

            trailStart->set_name(il2cpp_utils::createcsstr("TrailStart"));
            trailEnd->set_name(il2cpp_utils::createcsstr("TrailEnd"));
            UnityEngine::Object::SetName(trailStart,il2cpp_utils::createcsstr("TrailStart"));
            UnityEngine::Object::SetName(trailEnd,il2cpp_utils::createcsstr("TrailEnd"));

            if (model->get_gameObject()->GetComponents<TrickSaber::TrickSaberTrailData*>()->Length() == 0) {
                auto *trail = model->get_gameObject()->AddComponent<TrickSaber::TrickSaberTrailData *>();
                trail->Init(trailEnd->get_transform(), trailStart->get_transform(), obj);
            }

//            obj->set_enabled(false);

        }
    }

    void ChangeToTrickModel() {
        if (getPluginConfig().EnableTrickCutting.GetValue()) return;
        TrickModel->SetActive(true);

        auto* trickT = TrickModel->get_transform(); // CRASH_UNLESS(il2cpp_utils::GetPropertyValue(TrickModel, "transform"));
        auto* origT = OriginalSaberModel->get_transform(); // CRASH_UNLESS(il2cpp_utils::GetPropertyValue(OriginalSaberModel, "transform"));
        auto pos = origT->get_position(); //CRASH_UNLESS(il2cpp_utils::GetPropertyValue<Vector3>(origT, "position"));
        auto rot = origT->get_rotation(); //CRASH_UNLESS(il2cpp_utils::GetPropertyValue<Quaternion>(origT, "rotation"));

        trickT->set_position(pos);
        trickT->set_rotation(rot);
        OriginalSaberModel->SetActive(false);

        SaberGO = TrickModel;
    }

    void ChangeToActualSaber() {
        if (getPluginConfig().EnableTrickCutting.GetValue()) return;

        OriginalSaberModel->SetActive(true);
        TrickModel->SetActive(false);

        SaberGO = OriginalSaberModel;
    }

    // Sira utils methods
    void OverrideColor(GlobalNamespace::SetSaberGlowColor* ssgc, UnityEngine::Color color) {
        UnityEngine::MeshRenderer* mesh = ssgc->meshRenderer;
        UnityEngine::MaterialPropertyBlock* block = ssgc->materialPropertyBlock;
        Array<GlobalNamespace::SetSaberGlowColor::PropertyTintColorPair *> *tintPairs = ssgc->propertyTintColorPairs;
        if (block == nullptr)
        {
            block = new UnityEngine::MaterialPropertyBlock();
        }
        for (int i = 0; i < tintPairs->Length(); i++)
        {
            GlobalNamespace::SetSaberGlowColor::PropertyTintColorPair *ptcp = tintPairs->values[i];
            block->SetColor(ptcp->property, color * ptcp->tintColor);
        }
        mesh->SetPropertyBlock(block);
    }

    void OverrideColor(GlobalNamespace::SetSaberFakeGlowColor* ssfgc, UnityEngine::Color color) {
        GlobalNamespace::Parametric3SliceSpriteController* sliceSpriteController = ssfgc->parametric3SliceSprite;
        sliceSpriteController->color = color * ssfgc->tintColor;
        sliceSpriteController->Refresh();
    }



    void ChangeColorCoroutine2(UnityEngine::Color color) {
        getLogger().debug("Coloring saber model %d", (int) saberScript->get_saberType());

        auto *modelController = TrickModel->GetComponentInChildren<GlobalNamespace::SaberModelController *>(true);


//        if (modelController is IColorable colorable)
//        {
//            colorable.SetColor(color);
//        }

        auto tintColor = modelController->initData->trailTintColor;
        Array<GlobalNamespace::SetSaberGlowColor *> *setSaberGlowColors = modelController->setSaberGlowColors;
        Array<GlobalNamespace::SetSaberFakeGlowColor *> *setSaberFakeGlowColors = modelController->setSaberFakeGlowColors;

        modelController->saberTrail->color = (color * tintColor).get_linear();

        for (int i = 0; i != setSaberGlowColors->Length(); i++) {
            OverrideColor(setSaberGlowColors->values[i], color);
        }
        for (int i = 0; i < setSaberFakeGlowColors->Length(); i++) {
            OverrideColor(setSaberFakeGlowColors->values[i], color);
        }

        GlobalNamespace::TubeBloomPrePassLight *saberLight = modelController->saberLight;


        if (saberLight) {
            saberLight->color = color;
        } else {
            getLogger().debug("Saber light is null, should be normal right?");
        }
    }

    // Update every 5th frame
    void Update() {
        if (!Modloader::getMods().contains("Chroma")) return;

        if (TrickModel->get_activeSelf()) {

            update++;
            if (update == 5)
                update = 0;

            if (update == 0) {
                if (colorManager == nullptr)
                    colorManager = saberScript->GetComponentInChildren<GlobalNamespace::ColorManager *>();

                std::string findEnv = &"Chroma_colorSaber" [(int) saberScript->get_saberType()];

                auto result = getenv(findEnv.c_str());

                getLogger().debug("Got the result %s", result);

                if (result && strcmp(result, "true") == 0 && colorManager != nullptr) {
                    ChangeColorCoroutine2(colorManager->ColorForSaberType(saberScript->get_saberType()));
                }
            }
        } else update = 0;
    }

  private:
    UnityEngine::GameObject* OriginalSaberModel = nullptr;  // GameObject
    UnityEngine::GameObject* TrickModel = nullptr;          // GameObject
};
