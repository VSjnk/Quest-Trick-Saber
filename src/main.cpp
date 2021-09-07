#include "../include/main.hpp"
#include "../include/PluginConfig.hpp"
#include "../include/TrickManager.hpp"
#include "ConfigEnums.hpp"
#include "UnityEngine/Resources.hpp"
#include "GlobalNamespace/GameSongController.hpp"
#include "GlobalNamespace/BeatmapObjectSpawnController.hpp"
#include "GlobalNamespace/BeatmapObjectManager.hpp"
#include "UnityEngine/SceneManagement/SceneManager.hpp"
#include "GlobalNamespace/GamePause.hpp"
#include "GlobalNamespace/OculusVRHelper.hpp"
#include "GlobalNamespace/SaberBurnMarkArea.hpp"

#include "GlobalNamespace/ObstacleSaberSparkleEffectManager.hpp"
#include "custom-types/shared/register.hpp"

#include "GlobalNamespace/GameScenesManager.hpp"
#include "System/Action.hpp"
#include "System/Action_1.hpp"
#include "Zenject/DiContainer.hpp"
#include "GlobalNamespace/ScenesTransitionSetupDataSO.hpp"
#include "GlobalNamespace/SaberMovementData.hpp"
#include "UnityEngine/Events/UnityAction.hpp"
#include "HMUI/Touchable.hpp"
#include "GlobalNamespace/TimeHelper.hpp"
#include "TMPro/TextMeshPro.hpp"
#include "questui/shared/CustomTypes/Components/Backgroundable.hpp"

#include "UnityEngine/Canvas.hpp"
#include "GlobalNamespace/PauseMenuManager.hpp"
#include "GlobalNamespace/LevelBar.hpp"

#include "sombrero/shared/Vector2Utils.hpp"

#include "questui/shared/CustomTypes/Components/ExternalComponents.hpp"
#include "bs-utils/shared/utils.hpp"
#include <chrono>

#include <cstdlib>

#include <string>

#include "questui_components/shared/components/ViewComponent.hpp"
#include "questui_components/shared/components/Text.hpp"
#include "questui_components/shared/components/ScrollableContainer.hpp"
#include "questui_components/shared/components/HoverHint.hpp"
#include "questui_components/shared/components/Button.hpp"
#include "questui_components/shared/components/Modal.hpp"
#include "questui_components/shared/components/layouts/VerticalLayoutGroup.hpp"
#include "questui_components/shared/components/layouts/HorizontalLayoutGroup.hpp"
#include "questui_components/shared/components/settings/ToggleSetting.hpp"
#include "questui_components/shared/components/settings/StringSetting.hpp"
#include "questui_components/shared/components/settings/IncrementSetting.hpp"
#include "questui_components/shared/components/settings/DropdownSetting.hpp"

#include "ui/SeparatorLine.hpp"
#include "ui/TitleSectText.hpp"

#include "questui/shared/CustomTypes/Components/MainThreadScheduler.hpp"

#ifdef HAS_CODEGEN

#define AddConfigValueIncrementEnum(parent, enumConfigValue, enumClass, enumMap) BeatSaberUI::CreateIncrementSetting(parent, enumConfigValue.GetName() + " " + enumMap.at(clamp(0, (int) enumMap.size() - 1, (int) enumConfigValue.GetValue())), 0, 1, (float) clamp(0, (int) enumMap.size() - 1, (int) enumConfigValue.GetValue()), 0,(int) enumMap.size() - 1, il2cpp_utils::MakeDelegate<UnityEngine::Events::UnityAction_1<float>*>(classof(UnityEngine::Events::UnityAction_1<float>*), (void*)nullptr, +[](float value) { enumConfigValue.SetValue((int)value); }))

#define AddConfigValueDropdownEnum(parent, enumConfigValue, enumClass, enumMap, enumReverseMap) BeatSaberUI::CreateDropdown(parent, enumConfigValue.GetName(), enumReverseMap.at(clamp(0, (int) enumMap.size() - 1, (int) enumConfigValue.GetValue())), getKeys(enumMap), [](const std::string& value) {enumConfigValue.SetValue((int) enumMap.at(value));} )


#endif

using namespace GlobalNamespace;
using namespace QuestUI;
using namespace QuestUI_Components;
using namespace TrickSaberUI;


int clamp(int min, int max, int current) {
    if (current < min) return min;
    if (current > max) return max;

    return current;
}

// Returns a logger, useful for printing debug messages
Logger& getLogger() {
    static auto* logger = new Logger(modInfo, LoggerOptions(false, true));
    return *logger;
}



Configuration& getConfig() {
    static Configuration configuration(modInfo);
    return configuration;
}

extern "C" void setup(ModInfo& info) {
    info.id      = "TrickSaber";
    info.version = VERSION;
    modInfo      = info;
    getConfig().Load();
    getPluginConfig().Init(info);
    getConfig().Reload();
    getConfig().Write();

    getLogger().info("Leaving setup!");
}

//Saber* FakeSaber = nullptr;
//Saber* RealSaber = nullptr;
MAKE_HOOK_MATCH(SceneManager_SetActiveScene, &UnityEngine::SceneManagement::SceneManager::SetActiveScene, bool, UnityEngine::SceneManagement::Scene scene) {
    // Burn mark crash fix on song end
    EnableBurnMarks(0, true);
    EnableBurnMarks(1, true);
    return SceneManager_SetActiveScene(scene);
}

MAKE_HOOK_MATCH(SceneManager_Internal_SceneLoaded, &UnityEngine::SceneManagement::SceneManager::Internal_SceneLoaded, void, UnityEngine::SceneManagement::Scene scene, UnityEngine::SceneManagement::LoadSceneMode mode) {
    getLogger().info("SceneManager_Internal_SceneLoaded");
    if (auto* nameOpt = scene.get_name()) {
        auto* name = nameOpt;
        auto str = to_utf8(csstrtostr(name));
        getLogger().debug("Scene name internal: %s", str.c_str());
    }
    //FakeSaber = nullptr;
    //RealSaber = nullptr;
    TrickManager::StaticClear();
    leftSaber.Clear();
    rightSaber.Clear();
    SceneManager_Internal_SceneLoaded(scene, mode);
}

MAKE_HOOK_MATCH(GameScenesManager_PushScenes, &GlobalNamespace::GameScenesManager::PushScenes, void, GlobalNamespace::GameScenesManager* self, GlobalNamespace::ScenesTransitionSetupDataSO* scenesTransitionSetupData,
                     float minDuration, System::Action* afterMinDurationCallback,
                     System::Action_1<Zenject::DiContainer*>* finishCallback) {
    getLogger().debug("GameScenesManager_PushScenes");
    GameScenesManager_PushScenes(self, scenesTransitionSetupData, minDuration, afterMinDurationCallback, finishCallback);
    getConfig().Reload();

    if (getPluginConfig().EnableTrickCutting.GetValue() || getPluginConfig().SlowmoDuringThrow.GetValue()) {
        bs_utils::Submission::disable(modInfo);
    } else {
        bs_utils::Submission::enable(modInfo);
    }

    objectCount = 0;

    getLogger().debug("Leaving GameScenesManager_PushScenes");
}


static ViewComponent *view = nullptr;
static PauseMenuManager *loadedMenu = nullptr;
MAKE_HOOK_MATCH(PauseMenuManager_Start, &PauseMenuManager::Start, void, PauseMenuManager* self) {
    PauseMenuManager_Start(self);
    // trick saber pause manager UI

    if (self->levelBar) {

        getLogger().debug("Going to do pause menu");
        auto canvas = self->levelBar
                ->get_transform()
                ->get_parent()
                ->get_parent()
                ->GetComponent<UnityEngine::Canvas *>();
        if (!canvas) return;


        if (loadedMenu != self || !view) {
            loadedMenu = self;
            getLogger().debug("Creating view");

            getLogger().debug("Creating toggle");
            auto toggle = new QuestUI_Components::ConfigUtilsToggleSetting(getPluginConfig().TricksEnabled);

            view = new ViewComponent(canvas->get_transform(), {
                    toggle
            });
            view->render();

            auto* rectTransform = toggle->getTransform()->get_parent()->GetComponent<UnityEngine::RectTransform*>();

            CRASH_UNLESS(rectTransform);
            rectTransform->set_anchoredPosition({26, -15});
            rectTransform->set_sizeDelta({-130, 7});

            toggle->getTransform()->SetParent(self->levelBar->get_transform(), true);


            getLogger().debug("Finished pause menu");
        }
    }
}

MAKE_HOOK_MATCH(PauseMenuManager_OnDestroy, &PauseMenuManager::OnDestroy, void, PauseMenuManager* self) {
    PauseMenuManager_OnDestroy(self);
    getLogger().debug("Deleting view");
    delete view;
    view = nullptr;
    getLogger().debug("Deleted the view due to pause menu gone!");
}

MAKE_HOOK_MATCH(SaberManager_Start, &SaberManager::Start, void, SaberManager* self) {
    SaberManager_Start(self);
    saberManager = self;
    getLogger().debug("SaberManager_Start");

    if (to_utf8(csstrtostr(self->get_name())).starts_with(saberPrefix)) {
        getLogger().debug("Ignoring trick start");
        return;
    }

    // saber->sabertypeobject->sabertype
    //SaberType saberType = self->saberType->saberType;
    //getLogger().debug("SaberType: %i", (int) saberType);

    auto *vrControllers = UnityEngine::Resources::FindObjectsOfTypeAll<VRController*>();


    getLogger().info("VR controllers: %i", (int) vrControllers->Length());



    //if ((int) saberType == 0) {
        getLogger().debug("Left?");
//        leftSaber.VRController = vrControllersManager->get_node();
        leftSaber.Saber = self->leftSaber;
        leftSaber._isLeftSaber = true;
        leftSaber.other = &rightSaber;
        leftSaber.Start();
    //} else {
        getLogger().debug("Right?");
//        rightSaber.VRController = vrControllersManager->rightVRController;
        rightSaber.Saber = self->rightSaber;
        rightSaber.other = &leftSaber;
        rightSaber.Start();
    //}
    //RealSaber = self;
}



MAKE_HOOK_MATCH(Saber_ManualUpdate, &Saber::ManualUpdate, void, Saber* self) {
    TrickManager& trickManager = self == leftSaber.Saber ? leftSaber : rightSaber;

    Saber_ManualUpdate(self);

    if (!getPluginConfig().EnableTrickCutting.GetValue()) {
        static bool QosmeticsLoaded = Modloader::getMods().contains("Qosmetics");

        // Fix trails
        // If Qosmetics is on, we do not handle trails.
        if (trickManager.getTrickModel() &&

        // Only manipulate trail movement data
        trickManager.getTrickModel()->trailMovementData
        ) {
            auto trickModel = trickManager.getTrickModel();

            // Check if transforms have been made yet
            auto bottomTransform = trickManager.getTrickModel()->getModelBottomTransform();

            if (!QosmeticsLoaded && bottomTransform) {
                trickModel->trailMovementData->AddNewData(trickModel->getModelTopTransform()->get_position(),
                                                          bottomTransform->get_position(), TimeHelper::get_time());
            }

            auto trickSaberScript = trickManager.getTrickModel()->trickSaberScript;
            if (trickSaberScript && trickManager.isDoingTricks())
                SaberManualUpdate((Saber *) trickSaberScript);
        }
    }

    trickManager.Update();
}

static std::vector<System::Type*> tBurnTypes;


void DisableBurnMarks(int saberType) {
    TrickManager& trickManager = saberType == 0 ? leftSaber : rightSaber;

    if (!trickManager.getTrickModel())
        return;

    auto saberScript = trickManager.getTrickModel()->trickSaberScript;

    for (auto* type : tBurnTypes) {
        auto *components = UnityEngine::Object::FindObjectsOfType(type);
        if (!components)
            continue;

        for (il2cpp_array_size_t i = 0; i < components->Length(); i++) {
            auto *sabers = CRASH_UNLESS(il2cpp_utils::GetFieldValue<Array<Saber *> *>(components->values[i], "_sabers"));
            if (!sabers)
                continue;

            sabers->values[saberType] = (Saber *) saberScript;
        }
    }
}

void SaberManualUpdate(GlobalNamespace::Saber* saber) {
    TrickManager& trickManager = saber->saberType->saberType == 0 ? leftSaber : rightSaber;

    auto trickModel = trickManager.getTrickModel();

    if (!trickModel)
        return;


    saber->handlePos = saber->handleTransform->get_position();
    saber->handleRot = saber->handleTransform->get_rotation();

    auto bottomTransform = trickModel->getModelBottomTransform();
    auto topTransform = trickModel->getModelTopTransform();

    saber->saberBladeTopPos = topTransform->get_position();
    saber->saberBladeBottomPos = bottomTransform->get_position();
}

void EnableBurnMarks(int saberType, bool force) {
    TrickManager& trickManager = saberType == 0 ? leftSaber : rightSaber;

    if (!force && trickManager.isDoingTricks())
        return;

    for (auto *type : tBurnTypes) {
        auto *components = UnityEngine::Object::FindObjectsOfType(type);
        if (!components)
            continue;

        for (int i = 0; i < components->Length(); i++) {
            getLogger().debug("Burn Type: %s", to_utf8(csstrtostr(components->values[i]->get_name())).c_str());
            auto *sabers = CRASH_UNLESS(il2cpp_utils::GetFieldValue<Array<Saber *> *>(components->values[i], "_sabers"));

            if (!sabers) continue;

            sabers->values[saberType] = saberType ? saberManager->rightSaber : saberManager->leftSaber;
        }
    }
}

int64_t getTimeMillis() {
    auto time = std::chrono::high_resolution_clock::now();

    return std::chrono::duration_cast<std::chrono::milliseconds>(time.time_since_epoch()).count();
}


MAKE_HOOK_MATCH(FixedUpdate, &OculusVRHelper::FixedUpdate, void, GlobalNamespace::OculusVRHelper* self) {
    FixedUpdate(self);
    TrickManager::StaticFixedUpdate();
}

MAKE_HOOK_MATCH(Pause, &GamePause::Pause, void, GlobalNamespace::GamePause* self) {
    leftSaber.PauseTricks();
    rightSaber.PauseTricks();
    Pause(self);
    TrickManager::StaticPause();
    getLogger().debug("pause: %i", self->pause);
}

MAKE_HOOK_MATCH(Resume, &GamePause::Resume, void, GlobalNamespace::GamePause* self) {
    Resume(self);
    TrickManager::StaticResume();
    leftSaber.ResumeTricks();
    rightSaber.ResumeTricks();
    getLogger().debug("pause: %i", self->pause);
}

MAKE_HOOK_MATCH(AudioTimeSyncController_Start, &AudioTimeSyncController::Start, void, AudioTimeSyncController* self) {
    AudioTimeSyncController_Start(self);
    audioTimeSyncController = self;
    getLogger().debug("audio time controller: %p", audioTimeSyncController);
}

MAKE_HOOK_MATCH(SaberClashChecker_AreSabersClashing, &SaberClashChecker::AreSabersClashing, bool, SaberClashChecker* self, ByRef<UnityEngine::Vector3> clashingPoint) {
    if (!getPluginConfig().EnableTrickCutting.GetValue()) {
        bool val = SaberClashChecker_AreSabersClashing(self, clashingPoint);

        return (!rightSaber.isDoingTricks() && !leftSaber.isDoingTricks()) && val;
    } else {
        return SaberClashChecker_AreSabersClashing(self, clashingPoint);
    }
}

MAKE_HOOK_MATCH(VRController_Update, &VRController::Update, void, GlobalNamespace::VRController* self) {
    VRController_Update(self);

    if (self->get_node() == UnityEngine::XR::XRNode::LeftHand) {
        leftSaber.VRController = self;
    }

    if (self->get_node() == UnityEngine::XR::XRNode::RightHand) {
        rightSaber.VRController = self;
    }
}


MAKE_HOOK_MATCH(SpawnNote, &BeatmapObjectSpawnController::SpawnBasicNote, void, BeatmapObjectSpawnController* self, GlobalNamespace::NoteData* noteData, float cutDirectionAngleOffset) {
    if (getPluginConfig().NoTricksWhileNotes.GetValue())
        objectCount++;
//    getLogger().debug("Object count note increase %d", objectCount);

    SpawnNote(self, noteData, cutDirectionAngleOffset);
}

MAKE_HOOK_MATCH(SpawnBomb, &BeatmapObjectSpawnController::SpawnBombNote, void, BeatmapObjectSpawnController* self, GlobalNamespace::NoteData* noteData) {
    if (getPluginConfig().NoTricksWhileNotes.GetValue() )
        objectCount++;
//    getLogger().debug("Object count bomb increase %d", objectCount);

    SpawnBomb(self, noteData);
}

MAKE_HOOK_MATCH(NoteCut, &BeatmapObjectManager::HandleNoteControllerNoteWasCut, void, BeatmapObjectManager* self, GlobalNamespace::NoteController* noteController, ByRef<GlobalNamespace::NoteCutInfo> noteCutInfo) {
    if (getPluginConfig().NoTricksWhileNotes.GetValue() ) {
        objectDestroyTimes.push_back(getTimeMillis());
//    getLogger().debug("Object count note cut decrease %d", objectCount);

        if (objectCount < 0) objectCount = 0;
    }

    NoteCut(self, noteController, noteCutInfo);
}

MAKE_HOOK_MATCH(NoteMissed, &BeatmapObjectManager::HandleNoteControllerNoteWasMissed, void, BeatmapObjectManager* self, GlobalNamespace::NoteController* noteController) {
    if (getPluginConfig().NoTricksWhileNotes.GetValue()) {
        objectDestroyTimes.push_back(getTimeMillis());
//    getLogger().debug("Object count decrease %d", objectCount);

        if (objectCount < 0) objectCount = 0;
    }

    NoteMissed(self, noteController);
}



template <class K, class V>
std::vector<K> getKeys(std::map<K, V> map) {
    std::vector<K> vector;
    for (auto it = map.begin(); it != map.end(); it++) {
        vector.push_back(it->first);
    }

    return vector;
}

UnityEngine::UI::LayoutElement* CreateSeparatorLine(UnityEngine::Transform* parent) {
    // TODO: Fix
    UnityEngine::GameObject* go = UnityEngine::GameObject::New_ctor();
    go->get_transform()->SetParent(parent);
    auto layoutElement = go->AddComponent<UnityEngine::UI::LayoutElement*>();
    layoutElement->set_preferredWidth(90.0f);
    layoutElement->set_preferredHeight(8.0f);

    auto rectTransform = go->GetComponent<UnityEngine::RectTransform*>();
    rectTransform->set_sizeDelta(UnityEngine::Vector2(90, 8.0f));
    rectTransform->set_anchoredPosition(UnityEngine::Vector2::get_zero());

    rectTransform->set_anchorMin(UnityEngine::Vector2(0.0f, 0.0f));
    rectTransform->set_anchorMax(UnityEngine::Vector2(1.0f, 1.0f));

//    go->AddComponent<Backgroundable*>()->ApplyBackground(il2cpp_utils::newcsstr("round-rect-panel"));

    go->AddComponent<TMPro::TextMeshPro*>()->set_text(il2cpp_utils::newcsstr("thing!"));

    return layoutElement;
}

void DidActivate(HMUI::ViewController* self, bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling){
    getLogger().info("DidActivate: %p, %d, %d, %d", self, firstActivation, addedToHierarchy, screenSystemEnabling);

    static ViewComponent* view;

    if(firstActivation) {

//#define SEPARATOR_LINE CreateSeparatorLine(textGrid->get_transform());
        if (view) {
            delete view;
            view = nullptr;
        }

        static const auto sectTextMult = 6.0f;
        static const auto sectText = Sombrero::FastVector2(60.0f, 10.0f) * sectTextMult;

        // async ui because this causes lag spike
        std::thread([self]{
            view = new ViewComponent(self->get_transform(), {
                new ScrollableContainer({
                    new Text("TrickSaber settings. Restart to avoid crashes or side-effects."),
                    new Text("Settings are saved when changed."),
                    new Text("Not all settings have been tested. Please use with caution."),

                    new TitleSectText("Toggles and switches for buttons."),
                    new QuestUI_Components::ConfigUtilsToggleSetting(getPluginConfig().TricksEnabled),
                    new ConfigUtilsToggleSetting(getPluginConfig().ReverseTrigger),
                    new ConfigUtilsToggleSetting(getPluginConfig().ReverseButtonOne),
                    new ConfigUtilsToggleSetting(getPluginConfig().ReverseButtonTwo),
                    new ConfigUtilsToggleSetting(getPluginConfig().ReverseGrip),
                    new ConfigUtilsToggleSetting(getPluginConfig().ReverseThumbstick),

                    new SeparatorLine(),
                    new TitleSectText("Preferences."),
                    new ConfigUtilsToggleSetting(getPluginConfig().NoTricksWhileNotes),
                    new ConfigUtilsToggleSetting(getPluginConfig().VibrateOnReturn),
                    new ConfigUtilsToggleSetting(getPluginConfig().IsVelocityDependent),
                    new ConfigUtilsToggleSetting(getPluginConfig().MoveWhileThrown),
                    new ConfigUtilsToggleSetting(getPluginConfig().CompleteRotationMode),
                    new ConfigUtilsToggleSetting(getPluginConfig().SlowmoDuringThrow),

                    new SeparatorLine(),
                    new TitleSectText("Numbers and math. Threshold values"),
                    new ConfigUtilsIncrementSetting(getPluginConfig().GripThreshold, 2, 0.01),
                    new ConfigUtilsIncrementSetting(getPluginConfig().ControllerSnapThreshold, 2, 0.01),
                    new ConfigUtilsIncrementSetting(getPluginConfig().ThumbstickThreshold, 2, 0.01),
                    new ConfigUtilsIncrementSetting(getPluginConfig().TriggerThreshold, 2, 0.01),

                    new SeparatorLine(),
                    new TitleSectText("Speed and velocity manipulation"),
                    new ConfigUtilsIncrementSetting(getPluginConfig().SpinSpeed, 2, 0.01),
                    new ConfigUtilsIncrementSetting(getPluginConfig().ThrowVelocity, 2, 0.01),
                    new ConfigUtilsIncrementSetting(getPluginConfig().ReturnSpeed, 2, 0.01),

                    new SeparatorLine(),
                    new TitleSectText("Technical numbers, please avoid."),
                    new ConfigUtilsIncrementSetting(getPluginConfig().SlowmoStepAmount, 2, 0.01),
                    new ConfigUtilsIncrementSetting(getPluginConfig().SlowmoAmount, 2, 0.01),
                    new ConfigUtilsIncrementSetting(getPluginConfig().VelocityBufferSize, 0, 1),
                    new SeparatorLine(),
                    new TitleSectText("Actions Remapping (UI is very funky here)"),
                    (new TitleSectText("Freeze throw freezes the saber while thrown"))->with([](Text* text) {
                        text->mutateData([](MutableTextData data){
                            data.fontSize = data.fontSize.value_or(0) * 0.7;
                            return data;
                        });
                    }),
                    new ConfigUtilsEnumDropdownSetting<TrickAction>(getPluginConfig().TriggerAction),
                    new ConfigUtilsEnumDropdownSetting<TrickAction>(getPluginConfig().ButtonOneAction),
                    new ConfigUtilsEnumDropdownSetting<TrickAction>(getPluginConfig().ButtonTwoAction),
                    new ConfigUtilsEnumDropdownSetting<TrickAction>(getPluginConfig().ButtonOneAction),
                    new ConfigUtilsEnumDropdownSetting<TrickAction>(getPluginConfig().GripAction),
                    new ConfigUtilsEnumDropdownSetting<TrickAction>(getPluginConfig().ThumbstickAction),
                    new TitleSectText("Misc"),
                    new ConfigUtilsEnumDropdownSetting<SpinDir>(getPluginConfig().SpinDirection),
                    new ConfigUtilsEnumDropdownSetting<ThumbstickDir>(getPluginConfig().ThumbstickDirection)
                    })
            });

            QuestUI::MainThreadScheduler::Schedule([]{
                view->render();
            });
        }).detach();


    } else {
        view->render();
    }
}

extern "C" void load() {
    il2cpp_functions::Init();
    // TODO: config menus
    getLogger().info("Installing hooks...");

    INSTALL_HOOK(getLogger(), PauseMenuManager_Start);
    INSTALL_HOOK(getLogger(), PauseMenuManager_OnDestroy);
    INSTALL_HOOK(getLogger(), SceneManager_Internal_SceneLoaded);
    INSTALL_HOOK(getLogger(), SceneManager_SetActiveScene);
    INSTALL_HOOK(getLogger(), GameScenesManager_PushScenes);
    INSTALL_HOOK(getLogger(), Saber_ManualUpdate);

    INSTALL_HOOK(getLogger(), FixedUpdate);
    // INSTALL_HOOK(LateUpdate, il2cpp_utils::FindMethod("", "SaberBurnMarkSparkles", "LateUpdate"));

    INSTALL_HOOK(getLogger(), Pause);
    INSTALL_HOOK(getLogger(), Resume);

    INSTALL_HOOK(getLogger(), AudioTimeSyncController_Start);
    INSTALL_HOOK(getLogger(), SaberManager_Start);

    INSTALL_HOOK(getLogger(), SaberClashChecker_AreSabersClashing);
//    INSTALL_HOOK(getLogger(), SaberClashEffect_Disable, il2cpp_utils::FindMethod("", "SaberClashEffect", "OnDisable"));

    INSTALL_HOOK(getLogger(), VRController_Update);


    INSTALL_HOOK(getLogger(), SpawnNote);
    INSTALL_HOOK(getLogger(), SpawnBomb);
    INSTALL_HOOK(getLogger(), NoteMissed);
    INSTALL_HOOK(getLogger(), NoteCut);

//    INSTALL_HOOK(getLogger(), VRController_Update, il2cpp_utils::FindMethodUnsafe("", "GameSongController", "StartSong", 1));

    tBurnTypes.push_back(csTypeOf(GlobalNamespace::SaberBurnMarkArea*));
    tBurnTypes.push_back(csTypeOf(GlobalNamespace::SaberBurnMarkSparkles*));
    tBurnTypes.push_back(csTypeOf(GlobalNamespace::ObstacleSaberSparkleEffectManager*));

    getLogger().info("Registering custom types");
    getLogger().info("Registered types");

    getLogger().info("Installed all hooks!");

    getLogger().info("Starting Settings UI installation...");

    QuestUI::Init();
    QuestUI::Register::RegisterModSettingsViewController(modInfo, DidActivate);
    getLogger().info("Successfully installed Settings UI!");
}
