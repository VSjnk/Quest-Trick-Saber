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
using namespace QUC;
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


static std::unordered_map<Il2CppClass*, FieldInfo*> sabersFieldInfo;
static std::unordered_map<System::Type*, ArrayW<UnityEngine::Object*>> disabledBurnmarks;

static std::vector<System::Type*> tBurnTypes;


void DisableBurnMarks(int saberType) {
    TrickManager const& trickManager = saberType == 0 ? leftSaber : rightSaber;

    if (!trickManager.getTrickModel())
        return;

    auto const& saberScript = trickManager.getTrickModel()->trickSaberScript;

    for (auto *type: tBurnTypes) {
        auto& components = disabledBurnmarks[type];

        if (!components)
            components = UnityEngine::Object::FindObjectsOfType(type);

        if (!components || components.size() == 0)
            continue;

        for (auto disabled: components) {
            getLogger().debug("Burn Type: %s", to_utf8(csstrtostr(disabled->get_name())).c_str());

            auto &sabersInfo = sabersFieldInfo[disabled->klass];
            if (!sabersInfo)
                sabersInfo = il2cpp_utils::FindField(disabled, "_sabers");

            auto sabers = CRASH_UNLESS(il2cpp_utils::GetFieldValue<Array<Saber *> *>(disabled, sabersInfo));

            if (!sabers) continue;

            sabers->values[saberType] = (Saber *) saberScript;
        }
    }
}

void EnableBurnMarks(int saberType, bool force) {
    TrickManager const& trickManager = saberType == 0 ? leftSaber : rightSaber;

    if (!force && trickManager.isDoingTricks())
        return;

    for (auto *type: tBurnTypes) {
        auto& components = disabledBurnmarks[type];

        if (!components || components.size() == 0) continue;

        for (auto disabled: components) {
            getLogger().debug("Burn Type: %s", to_utf8(csstrtostr(disabled->get_name())).c_str());
            auto &sabersInfo = sabersFieldInfo[disabled->klass];
            if (!sabersInfo)
                sabersInfo = il2cpp_utils::FindField(disabled, "_sabers");

            auto sabers = CRASH_UNLESS(il2cpp_utils::GetFieldValue<Array<Saber *> *>(disabled, sabersInfo));

            if (!sabers) continue;

            sabers->values[saberType] = saberType ? saberManager->rightSaber : saberManager->leftSaber;
        }
    }
}


//Saber* FakeSaber = nullptr;
//Saber* RealSaber = nullptr;
MAKE_HOOK_MATCH(SceneManager_SetActiveScene, &UnityEngine::SceneManagement::SceneManager::SetActiveScene, bool, UnityEngine::SceneManagement::Scene scene) {
    // Burn mark crash fix on song end
    disabledBurnmarks.clear();
    return SceneManager_SetActiveScene(scene);
}

MAKE_HOOK_MATCH(SceneManager_Internal_SceneLoaded, &UnityEngine::SceneManagement::SceneManager::Internal_SceneLoaded, void, UnityEngine::SceneManagement::Scene scene, UnityEngine::SceneManagement::LoadSceneMode mode) {
    getLogger().info("SceneManager_Internal_SceneLoaded");
    if (auto nameOpt = scene.get_name()) {
        auto name = nameOpt;
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

    if (getPluginConfig().EnableTrickCutting.GetValue() || getPluginConfig().SlowmoDuringThrow.GetValue()) {
        bs_utils::Submission::disable(modInfo);
    } else {
        bs_utils::Submission::enable(modInfo);
    }

    objectCount = 0;

    getLogger().debug("Leaving GameScenesManager_PushScenes");
}


static RenderContext pauseMenuCtx = nullptr;
static PauseMenuManager *loadedMenu = nullptr;
MAKE_HOOK_MATCH(PauseMenuManager_Start, &PauseMenuManager::Start, void, PauseMenuManager* self) {
    PauseMenuManager_Start(self);
    // trick saber pause manager UI

    if (self->levelBar && loadedMenu != self) {
        getLogger().debug("Going to do pause menu");
        auto canvas = self->levelBar
                ->get_transform()
                ->get_parent()
                ->get_parent()
                ->GetComponent<UnityEngine::Canvas *>();
        if (!canvas) return;

        loadedMenu = self;
        getLogger().debug("Creating view");
        pauseMenuCtx = RenderContext(canvas->get_transform());

        getLogger().debug("Creating toggle");
        static auto toggle = ConfigUtilsToggleSetting(getPluginConfig().TricksEnabled);

        auto toggleTransform = detail::renderSingle(toggle, pauseMenuCtx);

        auto *rectTransform = toggleTransform->get_parent()->GetComponent<UnityEngine::RectTransform *>();

        CRASH_UNLESS(rectTransform);
        rectTransform->set_anchoredPosition({26, -15});
        rectTransform->set_sizeDelta({-130, 7});

        toggleTransform->SetParent(self->levelBar->get_transform(), true);


        getLogger().debug("Finished pause menu");

    }
}

MAKE_HOOK_MATCH(PauseMenuManager_OnDestroy, &PauseMenuManager::OnDestroy, void, PauseMenuManager* self) {
    PauseMenuManager_OnDestroy(self);
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
//
//    auto vrControllers = UnityEngine::Resources::FindObjectsOfTypeAll<VRController*>();
//
//
//    getLogger().info("VR controllers: %i", (int) vrControllers.Length());




    getLogger().debug("Left?");
    leftSaber.Saber = self->leftSaber;
    leftSaber._isLeftSaber = true;
    leftSaber.other = &rightSaber;
    leftSaber.Start();

    getLogger().debug("Right?");
    rightSaber.Saber = self->rightSaber;
    rightSaber.other = &leftSaber;
    rightSaber.Start();
}

MAKE_HOOK_MATCH(HapticFeedbackController_Awake, &GlobalNamespace::HapticFeedbackController::Awake, void, HapticFeedbackController* self) {
    HapticFeedbackController_Awake(self);
    TrickManager::_hapticFeedbackController = self;
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

MAKE_HOOK_MATCH(AudioTimeSyncController_Awake, &AudioTimeSyncController::Start, void, AudioTimeSyncController* self) {
    AudioTimeSyncController_Awake(self);
    TrickManager::audioTimeSyncController = self;
    getLogger().debug("audio time controller: %p", self);
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

    auto node = self->get_node();

    if (node == UnityEngine::XR::XRNode::LeftHand) {
        leftSaber.VRController = self;
    }

    if (node == UnityEngine::XR::XRNode::RightHand) {
        rightSaber.VRController = self;
    }
}

MAKE_HOOK_MATCH(SpawnNote, &BeatmapObjectSpawnController::HandleNoteDataCallback, void, BeatmapObjectSpawnController *self, GlobalNamespace::NoteData *noteData)
{
    if (getPluginConfig().NoTricksWhileNotes.GetValue())
        objectCount++;
//    getLogger().debug("Object count note increase %d", objectCount);

    SpawnNote(self, noteData);
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

    static RenderContext ctx(nullptr);

    static const auto sectTextMult = 6.0f;
    static const auto sectText = Sombrero::FastVector2(60.0f, 10.0f) * sectTextMult;

    static ScrollableContainer tree(
            Text("TrickSaber settings. Restart to avoid crashes or side-effects."),
            Text("Settings are saved when changed."),
            Text("Not all settings have been tested. Please use with caution."),

            TitleSectText("Toggles and switches for buttons."),
            QUC::ConfigUtilsToggleSetting(getPluginConfig().TricksEnabled),
            ConfigUtilsToggleSetting(getPluginConfig().ReverseTrigger),
            ConfigUtilsToggleSetting(getPluginConfig().ReverseButtonOne),
            ConfigUtilsToggleSetting(getPluginConfig().ReverseButtonTwo),
            ConfigUtilsToggleSetting(getPluginConfig().ReverseGrip),
            ConfigUtilsToggleSetting(getPluginConfig().ReverseThumbstick),

            SeparatorLine(),
            TitleSectText("Preferences."),
            ConfigUtilsToggleSetting(getPluginConfig().NoTricksWhileNotes),
            ConfigUtilsToggleSetting(getPluginConfig().VibrateOnReturn),
            ConfigUtilsToggleSetting(getPluginConfig().IsVelocityDependent),
            ConfigUtilsToggleSetting(getPluginConfig().MoveWhileThrown),
            ConfigUtilsToggleSetting(getPluginConfig().CompleteRotationMode),
            ConfigUtilsToggleSetting(getPluginConfig().SlowmoDuringThrow),

            SeparatorLine(),
            TitleSectText("Numbers and math. Threshold values"),
            ConfigUtilsIncrementSetting(getPluginConfig().GripThreshold, nullptr, 2, 0.01),
            ConfigUtilsIncrementSetting(getPluginConfig().ControllerSnapThreshold, nullptr, 2, 0.01),
            ConfigUtilsIncrementSetting(getPluginConfig().ThumbstickThreshold, nullptr, 2, 0.01),
            ConfigUtilsIncrementSetting(getPluginConfig().TriggerThreshold, nullptr, 2, 0.01),

            SeparatorLine(),
            TitleSectText("Speed and velocity manipulation"),
            ConfigUtilsIncrementSetting(getPluginConfig().SpinSpeed, nullptr, 2, 0.01),
            ConfigUtilsIncrementSetting(getPluginConfig().ThrowVelocity, nullptr, 2, 0.01),
            ConfigUtilsIncrementSetting(getPluginConfig().ReturnSpeed, nullptr, 2, 0.01),

            SeparatorLine(),
            TitleSectText("Technical numbers, please avoid."),
            ConfigUtilsIncrementSetting(getPluginConfig().SlowmoStepAmount, nullptr, 2, 0.01),
            ConfigUtilsIncrementSetting(getPluginConfig().SlowmoAmount, nullptr, 2, 0.01),
            ConfigUtilsIncrementSetting(getPluginConfig().VelocityBufferSize, nullptr, 0, 1),
            SeparatorLine(),
            TitleSectText("Actions Remapping (UI is very funky here)"),
            TitleSectText("Freeze throw freezes the saber while thrown", SECT_TEXT_MULT * 0.7),
            ConfigUtilsEnumDropdownSetting<TrickAction, TRICK_ACTION_COUNT>(getPluginConfig().TriggerAction),
            ConfigUtilsEnumDropdownSetting<TrickAction, TRICK_ACTION_COUNT>(getPluginConfig().ButtonOneAction),
            ConfigUtilsEnumDropdownSetting<TrickAction, TRICK_ACTION_COUNT>(getPluginConfig().ButtonTwoAction),
            ConfigUtilsEnumDropdownSetting<TrickAction, TRICK_ACTION_COUNT>(getPluginConfig().GripAction),
            ConfigUtilsEnumDropdownSetting<TrickAction, TRICK_ACTION_COUNT>(getPluginConfig().ThumbstickAction),
            TitleSectText("Misc"),
            ConfigUtilsEnumDropdownSetting<SpinDir, SPIN_DIR_COUNT>(getPluginConfig().SpinDirection),
            ConfigUtilsEnumDropdownSetting<ThumbstickDir, THUMBSTICK_DIR_COUNT>(getPluginConfig().ThumbstickDirection)
    );

    if(firstActivation) {
        ctx = RenderContext(self->get_transform());
    }

    detail::renderSingle(tree, ctx);
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

    INSTALL_HOOK(getLogger(), AudioTimeSyncController_Awake);
    INSTALL_HOOK(getLogger(), HapticFeedbackController_Awake);
    INSTALL_HOOK(getLogger(), SaberManager_Start);

    INSTALL_HOOK(getLogger(), SaberClashChecker_AreSabersClashing);
//    INSTALL_HOOK(getLogger(), SaberClashEffect_Disable, il2cpp_utils::FindMethod("", "SaberClashEffect", "OnDisable"));

    INSTALL_HOOK(getLogger(), VRController_Update);


    INSTALL_HOOK(getLogger(), SpawnNote);
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

    bs_utils::Submission::disable(modInfo);
    bs_utils::Submission::enable(modInfo);

    // Eager loading to stop freezing on first song select
    if (getPluginConfig().EnableTrickCutting.GetValue() || getPluginConfig().SlowmoDuringThrow.GetValue()) {
        bs_utils::Submission::disable(modInfo);
    }
}
