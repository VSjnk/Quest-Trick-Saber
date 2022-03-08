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
#include "UnityEngine/AudioSource.hpp"
#include "GlobalNamespace/Parametric3SliceSpriteController.hpp"
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

#include "sombrero/shared/ColorUtils.hpp"


#include "chroma/shared/SaberAPI.hpp"

#include "qosmetics-api/shared/SaberAPI.hpp"
#include "qosmetics-api/shared/Components/TrailHelper.hpp"
#include "qosmetics-api/shared/Components/ColorComponent.hpp"

#include "System/Collections/Generic/List_1.hpp"
#include <string>

static const std::string saberPrefix = "trick_saber_";

class SaberTrickModel {
  public:
    UnityEngine::Rigidbody* Rigidbody = nullptr;
    UnityEngine::GameObject* SaberGO;  // GameObject
    GlobalNamespace::Saber* saberScript;
    SafePtr<GlobalNamespace::Saber> trickSaberScript;

    GlobalNamespace::SaberMovementData* trailMovementData;

    bool basicSaber;

    [[nodiscard]] UnityEngine::Transform* getModelTopTransform() const {
        if (SaberGO == OriginalSaberModel && SaberGO) {
            if (originalTopPosParented)
                return originalTopPosParented->get_transform();


            return saberScript ? saberScript->saberBladeTopTransform : nullptr;
        } else {
            return trickTopPos->get_transform();
        }
    }

    [[nodiscard]] UnityEngine::Transform* getModelBottomTransform() const {
        if (SaberGO == OriginalSaberModel && SaberGO) {
            if (originalBottomPosParented)
                return originalBottomPosParented->get_transform();

            return saberScript ? saberScript->saberBladeBottomTransform : nullptr;
        } else {
            return trickBottomPos->get_transform();
        }
    }

    [[nodiscard]] UnityEngine::GameObject* getTrickModel() const {
        return this->TrickModel;
    }

    [[nodiscard]] UnityEngine::GameObject* getOriginalModel() const {
        return this->OriginalSaberModel;
    }

    [[nodiscard]] UnityEngine::GameObject* getActiveModel() const {
        return SaberGO;
    }

    SaberTrickModel(GlobalNamespace::Saber* saber, UnityEngine::GameObject* SaberModel, bool basicSaber) {
        CRASH_UNLESS(SaberModel);
        this->basicSaber = basicSaber;
        saberScript = saber;
        getLogger().debug("SaberTrickModel construction! %s", basicSaber ? "true" : "false");
        // il2cpp_utils::LogClass(il2cpp_functions::class_from_system_type(tRigidbody), false);

        SaberGO = OriginalSaberModel = SaberModel;
        origSaberModelController = OriginalSaberModel->get_transform()->get_parent()->GetComponentInChildren<GlobalNamespace::SaberModelController*>(true);

        if (getPluginConfig().EnableTrickCutting.GetValue()) {
            Rigidbody = SaberModel->GetComponent<UnityEngine::Rigidbody*>();
            if (!Rigidbody) {
                getLogger().warning("Adding rigidbody to original SaberModel?!");
                Rigidbody = SaberModel->AddComponent<UnityEngine::Rigidbody*>();
            }
            TrickModel = SaberGO;
            SetupRigidbody(Rigidbody, OriginalSaberModel);
        } else {
            auto createCustom = Qosmetics::SaberAPI::GetDummySaber(saberScript->saberType->saberType);

            // If qosmetics is not found or if it is default saber, create custom model ourselves
            if (!createCustom || !createCustom.value()) {
                getLogger().debug("Creating model manually");
                TrickModel = UnityEngine::Object::Instantiate(SaberModel);
                isCustom = !basicSaber;
            } else {
                getLogger().debug("Qosmetics made the model for us");
                TrickModel = createCustom.value()->get_gameObject();
                isCustom = true;
                colorComponent = std::make_optional(TrickModel->GetComponent<Qosmetics::ColorComponent*>());
            }


            CRASH_UNLESS(TrickModel);
            auto* newName = il2cpp_utils::newcsstr(
                    saberPrefix + to_utf8(csstrtostr(SaberModel->get_name())) + "_" + std::to_string((int) saber->saberType->saberType)
            );
            UnityEngine::Object::SetName(TrickModel, newName);
            getLogger().debug("Trick model name: %s", to_utf8(csstrtostr(TrickModel->get_name())).c_str());
            FixBasicTrickSaber(TrickModel, basicSaber);
            FixTransforms(basicSaber);
            AddTrickRigidbody();



            auto* str = CRASH_UNLESS(il2cpp_utils::newcsstr("VRGameCore"));
            auto* vrGameCore = UnityEngine::GameObject::Find(str);
            auto* vrGameCoreT = vrGameCore->get_transform();
            auto* trickModelT = TrickModel->get_transform();

            trickModelT->SetParent(vrGameCoreT);
            SetActive(TrickModel,false);
            SetActive(OriginalSaberModel, true);

            auto origModelController = OriginalSaberModel->GetComponent<GlobalNamespace::SaberModelController*>();

            // Only color if no one else is coloring the main saber or the trick saber
            getLogger().debug("Main saber %s Trick Saber %s", isOptional(Chroma::SaberAPI::isSaberColorable(origModelController)) ? "true" : "false", isOptional(Chroma::SaberAPI::isSaberColorable(trickSaberModelController)) ? "true" : "false");
            if (!isOptional(Chroma::SaberAPI::isSaberColorable(trickSaberModelController))) {
                auto callback = Chroma::SaberAPI::getSaberChangedColorCallbackSafe();

                if (callback) {
                    getLogger().debug("Registering to chroma!");
                    UnorderedEventCallback<int, GlobalNamespace::SaberModelController *, Sombrero::FastColor> &refCallback = *callback;
                    refCallback += {&SaberTrickModel::MarkRefreshColor, this};
                }
            }
        }

        getLogger().debug("Leaving SaberTrickModel construction!");
    }

    static bool isOptional(std::optional<bool> opt) {
        return opt && opt.value();
    }

    ~SaberTrickModel() {
        auto callback = Chroma::SaberAPI::getSaberChangedColorCallbackSafe();

        if (callback) {
            UnorderedEventCallback<int, GlobalNamespace::SaberModelController*, Sombrero::FastColor>& refCallback = *callback;
            refCallback -= {&SaberTrickModel::MarkRefreshColor, this};
        }
    }

    static void SetActive(UnityEngine::GameObject* go, bool active) {
        static bool QosmeticsLoaded = Modloader::getMods().contains("Qosmetics");
        static auto zero = UnityEngine::Vector3::get_zero();
        static auto away = UnityEngine::Vector3(999999, 999999, 999999);

        // We teleport the saber away so it cannot be seen while the animation loads
        // such as it is with the Katana sabers
        // We also reset/hide the trail so it doesn't look teleported away
        if (QosmeticsLoaded) {
            static auto altTrailKlass = il2cpp_utils::GetClassFromName("Qosmetics", "AltTrail");
            static auto TrailHelperKlass = il2cpp_utils::GetClassFromName("Qosmetics", "TrailHelper");

            if (altTrailKlass) {
                static auto altTrailType = il2cpp_utils::GetSystemType(altTrailKlass);
                static auto TrailHelperType = il2cpp_utils::GetSystemType(TrailHelperKlass);

                auto trails = go->GetComponentsInChildren(TrailHelperType, !active);
                go->get_transform()->set_localPosition(active ? zero : away);

                for (int i = 0; i < trails.Length(); i++) {
                    auto trail = reinterpret_cast<Qosmetics::TrailHelper *>(trails.get(i));


                    if (trail) {
//                    trail->set_enabled(active);

                        // Reset Qosmetics trail data
                        // Avoid often, so only when the saber is set to active
                        trail->SetTrailActive(active);
                        if (active && false) {
                            try {
                                static auto QosmeticsTrail_reset = il2cpp_utils::FindMethodUnsafe(altTrailKlass,
                                                                                                  "Reset",
                                                                                                  1);

                                if (QosmeticsTrail_reset) {
                                    getLogger().debug("Trail reset!");
//                                    il2cpp_utils::RunMethodThrow<void, false>(trail, QosmeticsTrail_reset, false);
                                } else {
                                    getLogger().error("Qosmetics trail code needs fixing!");
                                }
                            } catch (il2cpp_utils::Il2CppUtilsException &e) {
                                getLogger().error("Qosmetics trail code needs fixing! message: %s", e.msg.c_str());
                            }
                        }
                    }
                }
            }

        } else {
            go->SetActive(active);
        }
    }

    template<auto val>
    using FPtrWrapper = il2cpp_utils::il2cpp_type_check::FPtrWrapper<val>;

    void MarkRefreshColor(int saberType, GlobalNamespace::SaberModelController* model, Sombrero::FastColor color) {
        if (saberType != (int) this->saberScript->saberType->saberType || model != origSaberModelController) {
            getLogger().error("Coloring the wrong saber colored: %p and original %p ", model, origSaberModelController);
            return;
        } else {
            getLogger().info("Marking color!");
        }

        newColor = std::make_optional(color);

        if (SaberGO == TrickModel) {
            RefreshColor();
        }
    }

    // If only Chroma's Saber Colorizer could colorize this model
    void RefreshColor() {
        if (!newColor)
            return;

        auto color = *newColor;

        if (isCustom) {
            if (colorComponent && colorComponent.value()) {
                getLogger().debug("Updating with Qosmetics!");
                colorComponent.value()->UpdateColors();
            } else {
                getLogger().debug("Color component not found for custom saber!");
            }
        } else {
            getLogger().debug("Coloring the %i saber!", saberScript->saberType->saberType.value);
            static auto SetColor = FPtrWrapper<static_cast<void (UnityEngine::MaterialPropertyBlock::*)(StringW,
                    UnityEngine::Color)>(&UnityEngine::MaterialPropertyBlock::SetColor)>::get();
            static auto SetPropertyBlock = FPtrWrapper<static_cast<void (UnityEngine::Renderer::*)(
                    UnityEngine::MaterialPropertyBlock *)>(&UnityEngine::Renderer::SetPropertyBlock)>::get();
            static auto Refresh = FPtrWrapper<&GlobalNamespace::Parametric3SliceSpriteController::Refresh>::get();

            auto _setSaberGlowColors = trickSaberModelController->setSaberGlowColors;
            auto _setSaberFakeGlowColors = trickSaberModelController->setSaberFakeGlowColors;

            GlobalNamespace::SaberTrail *saberTrail = trickSaberModelController->saberTrail;
            const Sombrero::FastColor &_trailTintColor = trickSaberModelController->initData->trailTintColor;
            GlobalNamespace::TubeBloomPrePassLight *_saberLight = trickSaberModelController->saberLight;

            saberTrail->color = (color * _trailTintColor).Linear();

            if (_setSaberGlowColors) {
                for (int i = 0; i < _setSaberGlowColors.Length(); i++) {
                    auto setSaberGlowColor = _setSaberGlowColors.get(i);

                    if (!setSaberGlowColor)
                        continue;

                    UnityEngine::MaterialPropertyBlock *materialPropertyBlock = setSaberGlowColor->materialPropertyBlock;
                    if (!materialPropertyBlock) {
                        setSaberGlowColor->materialPropertyBlock = UnityEngine::MaterialPropertyBlock::New_ctor();
                        materialPropertyBlock = setSaberGlowColor->materialPropertyBlock;
                    }

                    auto propertyTintColorPairs = setSaberGlowColor->propertyTintColorPairs;

                    if (propertyTintColorPairs && propertyTintColorPairs.Length() > 0) {
                        for (auto &propertyTintColorPair : propertyTintColorPairs) {
                            if (propertyTintColorPair)
                                SetColor(materialPropertyBlock, propertyTintColorPair->property, color * propertyTintColorPair->tintColor);
                        }
                    }

                    if (setSaberGlowColor->meshRenderer)
                        SetPropertyBlock(setSaberGlowColor->meshRenderer, materialPropertyBlock);
                }
            }

            if (_setSaberFakeGlowColors) {
                for (int i = 0; i < _setSaberFakeGlowColors.Length(); i++) {
                    auto setSaberFakeGlowColor = _setSaberFakeGlowColors.get(i);
                    if (!setSaberFakeGlowColor) continue;

                    auto parametric3SliceSprite = setSaberFakeGlowColor->parametric3SliceSprite;
                    parametric3SliceSprite->color = color * setSaberFakeGlowColor->tintColor;
                    Refresh(parametric3SliceSprite);
                }
            }

            if (_saberLight) {
                _saberLight->color = color;
            }
        }

        getLogger().debug("Done coloring!");
        newColor = std::nullopt;

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

        auto colliders = model->GetComponentsInChildren<UnityEngine::Collider*>(true); //CRASH_UNLESS(il2cpp_utils::RunMethod<Array<Il2CppObject*>*>(model, "GetComponentsInChildren", tCollider, true));


        for (int i = 0; i < colliders.Length(); i++) {
            colliders.get(i)->set_enabled(false);
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

        GlobalNamespace::SaberModelContainer* oldSaberModelContainer = SaberGO->GetComponentsInParent<GlobalNamespace::SaberModelContainer*>(false).get(0);

        GlobalNamespace::SaberTypeObject* _saberTypeObject = oldSaberModelContainer->saber->saberType;
        GlobalNamespace::SaberType saberType = _saberTypeObject->saberType; // CRASH_UNLESS(il2cpp_utils::GetPropertyValue(_saberTypeObject, "saberType"));
        getLogger().debug("saber type: %i", (int) saberType);
//        CRASH_UNLESS(saberType);
        auto* oldSaberModelContainerT = oldSaberModelContainer->get_transform();
        getLogger().debug("saber container");
        CRASH_UNLESS(oldSaberModelContainerT);

        auto* newSaberModelController = newSaber->GetComponent<GlobalNamespace::SaberModelController*>();
        trickSaberModelController = newSaberModelController;
        getLogger().debug("saber controller");
        CRASH_UNLESS(newSaberModelController);

        auto* origModelController = SaberGO->GetComponent<GlobalNamespace::SaberModelController*>(); // CRASH_UNLESS(il2cpp_utils::RunMethod(SaberGO, "GetComponent", tSaberModelController));
        auto* origColorMgr = origModelController->colorManager; // CRASH_UNLESS(il2cpp_utils::GetFieldValue(origModelController, "_colorManager"));
        getLogger().debug("saber color manager");
        CRASH_UNLESS(origColorMgr);
        newSaberModelController->colorManager = origColorMgr;

        auto glows = newSaberModelController->setSaberGlowColors; // CRASH_UNLESS(il2cpp_utils::GetFieldValue<Il2CppArray*>(saberModelController, "_setSaberGlowColors"));
        getLogger().debug("_setSaberGlowColors.length: %i", (int) glows.Length());
        for (GlobalNamespace::SetSaberGlowColor* obj : glows) {
            obj->colorManager = origColorMgr;
        }

        auto fakeGlows = newSaberModelController->setSaberFakeGlowColors;
        getLogger().debug("_setSaberFakeGlowColors.length: %i", (int) fakeGlows.Length());
        for (int i = 0; i < fakeGlows.Length(); i++) {
            GlobalNamespace::SetSaberFakeGlowColor* obj = fakeGlows.get(i);

            obj->colorManager = origColorMgr;

        }



        static bool QosmeticsLoaded = Modloader::getMods().contains("Qosmetics");

        if (QosmeticsLoaded) {
            // Disable audio sources for Qosmetics sabers such as Katana sabers.
            auto audioSources = newSaberModelController->GetComponentsInChildren<UnityEngine::AudioSource*>(true);
            for (int i = 0; i < audioSources.Length(); i++) {
                auto source = audioSources.get(i);

                if (source) {
                    source->set_playOnAwake(false);
                }
            }
        }

        auto* trickT = TrickModel->get_transform(); // CRASH_UNLESS(il2cpp_utils::GetPropertyValue(TrickModel, "transform"));
        auto* origT = OriginalSaberModel->get_transform(); // CRASH_UNLESS(il2cpp_utils::GetPropertyValue(OriginalSaberModel, "transform"));

        auto pos = origT->get_position(); //CRASH_UNLESS(il2cpp_utils::GetPropertyValue<Vector3>(origT, "position"));
        auto rot = origT->get_rotation(); //CRASH_UNLESS(il2cpp_utils::GetPropertyValue<Quaternion>(origT, "rotation"));

        trickT->set_position(pos);
        trickT->set_rotation(rot);


        newSaberModelController->Init(oldSaberModelContainerT, oldSaberModelContainer->saber);

        newSaberModelController->saberTrail->color = origSaberModelController->saberTrail->color;
        newSaberModelController->initData->trailTintColor = origSaberModelController->initData->trailTintColor;
    }

    void FixTransforms(bool basicSaber) {
        // Fix trails
        auto *trailStart = UnityEngine::GameObject::New_ctor();
        auto *trailEnd = UnityEngine::GameObject::New_ctor();


        trailStart->get_transform()->SetPositionAndRotation(saberScript->saberBladeBottomTransform->get_position(),
                                                            saberScript->saberBladeBottomTransform->get_rotation());
        trailStart->get_transform()->set_position(saberScript->saberBladeBottomTransform->get_position());
        trailStart->get_transform()->SetParent(TrickModel->get_transform());

        trailEnd->get_transform()->SetPositionAndRotation(saberScript->saberBladeTopTransform->get_position(),
                                                          saberScript->saberBladeTopTransform->get_rotation());
        trailEnd->get_transform()->set_position(saberScript->saberBladeTopTransform->get_position());
        trailEnd->get_transform()->SetParent(TrickModel->get_transform());

        trickTopPos = trailEnd;
        trickBottomPos = trailStart;

        if (basicSaber && !Modloader::getMods().contains("Qosmetics")) {
            // Fix trails
            trailMovementData = GlobalNamespace::SaberMovementData::New_ctor();

            FixTrails(OriginalSaberModel);
            FixTrails(TrickModel);
        }

        // Create our own transforms that are parented
        auto bottomPos = UnityEngine::GameObject::New_ctor();
        auto topPos = UnityEngine::GameObject::New_ctor();

        bottomPos->get_transform()->SetParent(OriginalSaberModel->get_transform());
        topPos->get_transform()->SetParent(OriginalSaberModel->get_transform());

        bottomPos->get_transform()->SetPositionAndRotation(saberScript->saberBladeBottomTransform->get_position(),
                                                           saberScript->saberBladeBottomTransform->get_rotation());

        topPos->get_transform()->SetPositionAndRotation(saberScript->saberBladeTopTransform->get_position(),
                                                        saberScript->saberBladeTopTransform->get_rotation());




        originalBottomPosParented = bottomPos;
        originalTopPosParented = topPos;

        if (!getPluginConfig().EnableTrickCutting.GetValue()) {
            UnityEngine::GameObject* go = UnityEngine::GameObject::New_ctor(il2cpp_utils::newcsstr("TrickSaberScript" + std::to_string(saberScript->get_saberType().value)));
            trickSaberScript.emplace(go->AddComponent<GlobalNamespace::Saber*>());
            trickSaberScript->saberBladeBottomTransform = getModelBottomTransform();
            trickSaberScript->saberBladeTopTransform = getModelTopTransform();
            trickSaberScript->saberType = saberScript->saberType;
            trickSaberScript->movementData = trailMovementData;
            trickSaberScript->handleTransform = saberScript->handleTransform;
            SaberManualUpdate((GlobalNamespace::Saber *) trickSaberScript);
        }
    }

    void FixTrails(UnityEngine::GameObject* model) const {
        auto trails = model->GetComponentsInChildren<GlobalNamespace::SaberTrail*>(true);

        getLogger().debug("trick saber trails.length: %i", (int) trails.Length());
        for (int i = 0; i < trails.Length(); i++)  {
            GlobalNamespace::SaberTrail *obj = trails.get(i);

            obj->movementData = reinterpret_cast<GlobalNamespace::IBladeMovementData*>(trailMovementData);
        }
    }

    void ChangeToTrickModel() {
        if (getPluginConfig().EnableTrickCutting.GetValue()) return;
        SetActive(TrickModel, true);

        auto* trickT = TrickModel->get_transform(); // CRASH_UNLESS(il2cpp_utils::GetPropertyValue(TrickModel, "transform"));
        auto* origT = OriginalSaberModel->get_transform(); // CRASH_UNLESS(il2cpp_utils::GetPropertyValue(OriginalSaberModel, "transform"));
        auto pos = origT->get_position(); //CRASH_UNLESS(il2cpp_utils::GetPropertyValue<Vector3>(origT, "position"));
        auto rot = origT->get_rotation(); //CRASH_UNLESS(il2cpp_utils::GetPropertyValue<Quaternion>(origT, "rotation"));

        trickT->set_position(pos);
        trickT->set_rotation(rot);
        SetActive(OriginalSaberModel, false);

        RefreshColor();

        SaberGO = TrickModel;
    }

    void ChangeToActualSaber() {
        if (getPluginConfig().EnableTrickCutting.GetValue()) return;

        SetActive(OriginalSaberModel, true);
        SetActive(TrickModel, false);

        SaberGO = OriginalSaberModel;
    }

    // Sira utils methods
    void ChangeColorTrickModel(UnityEngine::Color color) {
        getLogger().debug("Coloring saber model %d", (int) saberScript->get_saberType());

        auto *modelController = trickSaberModelController;
        
        // fix for a (possibly Qosmetics?) bug where one of these caused a nullptr deref on getting the tint color, this early return should make it not crash
        if (!modelController) {
            getLogger().error("Didn't find modelcontroller in children, returning...");
            return;
        }
        else if (!modelController->initData) {
            getLogger().error("initData nullptr, returning...");
            return;
        }

        auto _setSaberGlowColors = modelController->setSaberGlowColors;
        auto _setSaberFakeGlowColors = modelController->setSaberFakeGlowColors;

        auto saberTrail = modelController->saberTrail;
        saberTrail->color = (color * trickSaberModelController->initData->trailTintColor).get_linear();

        if (_setSaberGlowColors) {
            for (int i = 0; i < _setSaberGlowColors.Length(); i++) {
                auto setSaberGlowColor = _setSaberGlowColors.get(i);

                if (!setSaberGlowColor)
                    continue;

                UnityEngine::MaterialPropertyBlock* materialPropertyBlock = setSaberGlowColor->materialPropertyBlock;
                if (!materialPropertyBlock) {
                    setSaberGlowColor->materialPropertyBlock = UnityEngine::MaterialPropertyBlock::New_ctor();
                    materialPropertyBlock = setSaberGlowColor->materialPropertyBlock;
                }

                auto propertyTintColorPairs = setSaberGlowColor->propertyTintColorPairs;

                if (propertyTintColorPairs && propertyTintColorPairs.Length() > 0) {
                    for (auto &propertyTintColorPair : propertyTintColorPairs) {
                        if (propertyTintColorPair)
                            materialPropertyBlock->SetColor(propertyTintColorPair->property,
                                                            color * propertyTintColorPair->tintColor);
                    }
                }

                if (setSaberGlowColor->meshRenderer)
                    setSaberGlowColor->meshRenderer->SetPropertyBlock((UnityEngine::MaterialPropertyBlock *) materialPropertyBlock);
            }
        }

        if (_setSaberFakeGlowColors) {
            for (int i = 0; i < _setSaberFakeGlowColors.Length(); i++) {
                auto setSaberFakeGlowColor = _setSaberFakeGlowColors.get(i);
                if (!setSaberFakeGlowColor) continue;

                auto parametric3SliceSprite = setSaberFakeGlowColor->parametric3SliceSprite;
                parametric3SliceSprite->color = color * setSaberFakeGlowColor->tintColor;
                parametric3SliceSprite->Refresh();
            }
        }

        auto saberLight = modelController->saberLight;

        if (saberLight)
        {
            saberLight->color = color;
        }
    }

  private:
    UnityEngine::GameObject* OriginalSaberModel = nullptr;  // GameObject
    UnityEngine::GameObject* TrickModel = nullptr;          // GameObject

    UnityEngine::GameObject *trickBottomPos;
    UnityEngine::GameObject *trickTopPos;

    UnityEngine::GameObject *originalBottomPosParented;
    UnityEngine::GameObject *originalTopPosParented;

    std::optional<Qosmetics::ColorComponent*> colorComponent;
    GlobalNamespace::SaberModelController* origSaberModelController;
    GlobalNamespace::SaberModelController* trickSaberModelController;

    bool isCustom;
    std::optional<Sombrero::FastColor> newColor = std::nullopt;

};
