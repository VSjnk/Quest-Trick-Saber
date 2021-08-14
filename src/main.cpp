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

#include "sombrero/shared/Vector2Utils.hpp"

#include "questui/shared/CustomTypes/Components/ExternalComponents.hpp"
#include "bs-utils/shared/utils.hpp"
#include <chrono>

#include <cstdlib>

#include <string>

#ifdef HAS_CODEGEN

#define AddConfigValueIncrementEnum(parent, enumConfigValue, enumClass, enumMap) BeatSaberUI::CreateIncrementSetting(parent, enumConfigValue.GetName() + " " + enumMap.at(clamp(0, (int) enumMap.size() - 1, (int) enumConfigValue.GetValue())), 0, 1, (float) clamp(0, (int) enumMap.size() - 1, (int) enumConfigValue.GetValue()), 0,(int) enumMap.size() - 1, il2cpp_utils::MakeDelegate<UnityEngine::Events::UnityAction_1<float>*>(classof(UnityEngine::Events::UnityAction_1<float>*), (void*)nullptr, +[](float value) { enumConfigValue.SetValue((int)value); }))

#define AddConfigValueDropdownEnum(parent, enumConfigValue, enumClass, enumMap, enumReverseMap) BeatSaberUI::CreateDropdown(parent, enumConfigValue.GetName(), enumReverseMap.at(clamp(0, (int) enumMap.size() - 1, (int) enumConfigValue.GetValue())), getKeys(enumMap), [](const std::string& value) {enumConfigValue.SetValue((int) enumMap.at(value));} )


#endif

using namespace GlobalNamespace;
using namespace QuestUI;


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

void EnableBurnMarks(int saberType) {
    TrickManager& trickManager = saberType == 0 ? leftSaber : rightSaber;

    if (trickManager.isDoingTricks())
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

    if(firstActivation) {
#define SEPARATOR_LINE BeatSaberUI::CreateStringSetting(textGrid->get_transform(), "", "")->set_interactable(false);
//#define SEPARATOR_LINE CreateSeparatorLine(textGrid->get_transform());


        self->get_gameObject()->AddComponent<HMUI::Touchable*>();
        UnityEngine::GameObject* container = BeatSaberUI::CreateScrollableSettingsContainer(self->get_transform());

        static const auto zero = Sombrero::FastVector2::zero();
        static const auto sectTextMult = 6.0f;
        static const auto sectText = Sombrero::FastVector2(60.0f, 10.0f) * sectTextMult;

        auto* textGrid = container;
//        textGrid->set_spacing(1);

        BeatSaberUI::CreateText(textGrid->get_transform(), "TrickSaber settings. Restart to avoid crashes or side-effects.");

        BeatSaberUI::CreateText(textGrid->get_transform(), "Settings are saved when changed.");

        BeatSaberUI::CreateText(textGrid->get_transform(), "Not all settings have been tested. Please use with caution.");

        SEPARATOR_LINE

//        buttonsGrid->set_spacing(1);

        auto* boolGrid = container;

        BeatSaberUI::CreateText(boolGrid->get_transform(), "Toggles and switches for buttons.", false)->set_fontSize(sectTextMult);


        BeatSaberUI::AddHoverHint(AddConfigValueToggle(boolGrid->get_transform(), getPluginConfig().ReverseTrigger)->get_gameObject(),"Inverts the trigger button");
        BeatSaberUI::AddHoverHint(AddConfigValueToggle(boolGrid->get_transform(), getPluginConfig().ReverseButtonOne)->get_gameObject(),"Inverts the button one toggle.");
        BeatSaberUI::AddHoverHint(AddConfigValueToggle(boolGrid->get_transform(), getPluginConfig().ReverseButtonTwo)->get_gameObject(),"Inverts the button two toggle.");
        BeatSaberUI::AddHoverHint(AddConfigValueToggle(boolGrid->get_transform(), getPluginConfig().ReverseGrip)->get_gameObject(),"Inverts the grip toggle.");
        BeatSaberUI::AddHoverHint(AddConfigValueToggle(boolGrid->get_transform(), getPluginConfig().ReverseThumbstick)->get_gameObject(),"Inverts the thumbstick direction.");

        SEPARATOR_LINE
        BeatSaberUI::CreateText(boolGrid->get_transform(), "Preferences.", false)->set_fontSize(sectTextMult);
        BeatSaberUI::AddHoverHint(AddConfigValueToggle(boolGrid->get_transform(), getPluginConfig().NoTricksWhileNotes)->get_gameObject(),"Doesn't allow tricks while notes are on screen");
        BeatSaberUI::AddHoverHint(AddConfigValueToggle(boolGrid->get_transform(), getPluginConfig().VibrateOnReturn)->get_gameObject(),"Makes the controller vibrate when it returns from being thrown");
        BeatSaberUI::AddHoverHint(AddConfigValueToggle(boolGrid->get_transform(), getPluginConfig().IsVelocityDependent)->get_gameObject(),"Makes the spin speed velocity dependent.");
        // TODO: Fix or remove
        //        BeatSaberUI::AddHoverHint(AddConfigValueToggle(boolGrid->get_transform(), getPluginConfig().EnableTrickCutting)->get_gameObject(),"Allows for physics to apply with the tricks. DOES NOT WORK AND IS VERY BROKEN.");
        BeatSaberUI::AddHoverHint(AddConfigValueToggle(boolGrid->get_transform(), getPluginConfig().CompleteRotationMode)->get_gameObject(),"Allows for the spin rotation to go all directions.");
        BeatSaberUI::AddHoverHint(AddConfigValueToggle(boolGrid->get_transform(), getPluginConfig().SlowmoDuringThrow)->get_gameObject(),"Makes the thrown saber act slow-mo like.");

        auto* floatGrid = container;
        SEPARATOR_LINE
        BeatSaberUI::CreateText(floatGrid->get_transform(), "Numbers and math. Threshold values", false)->set_fontSize(sectTextMult);
//        floatGrid->set_spacing(1);

        BeatSaberUI::AddHoverHint(AddConfigValueIncrementFloat(floatGrid->get_transform(), getPluginConfig().GripThreshold, 2, 0.01, 0, 1)->get_gameObject(),"The deadzone or minimum amount of input required to trigger the grip.");
        BeatSaberUI::AddHoverHint(AddConfigValueIncrementFloat(floatGrid->get_transform(), getPluginConfig().ControllerSnapThreshold, 2, 0.01, 0, 1)->get_gameObject(),"The deadzone or minimum amount of input required for the controller to snap.");
        BeatSaberUI::AddHoverHint(AddConfigValueIncrementFloat(floatGrid->get_transform(), getPluginConfig().ThumbstickThreshold, 2, 0.01, 0, 1)->get_gameObject(),"The deadzone or minimum amount of input required to trigger the thumbstick.");
        BeatSaberUI::AddHoverHint(AddConfigValueIncrementFloat(floatGrid->get_transform(), getPluginConfig().TriggerThreshold, 2, 0.01, 0, 1)->get_gameObject(),"The deadzone or minimum amount of input required to trigger.");

        SEPARATOR_LINE
        BeatSaberUI::CreateText(floatGrid->get_transform(), "Speed and velocity manipulation", false)->set_fontSize(sectTextMult);

        BeatSaberUI::AddHoverHint(AddConfigValueIncrementFloat(floatGrid->get_transform(), getPluginConfig().SpinSpeed, 1, 0.1, 0, 10)->get_gameObject(),"The speed the saber spins at.");
        BeatSaberUI::AddHoverHint(AddConfigValueIncrementFloat(floatGrid->get_transform(), getPluginConfig().ThrowVelocity, 1, 0.1, 0, 10)->get_gameObject(),"The velocity of the saber when you throw it.");
        BeatSaberUI::AddHoverHint(AddConfigValueIncrementFloat(floatGrid->get_transform(), getPluginConfig().ReturnSpeed, 1, 0.1, 0, 10)->get_gameObject(),"The speed in which the saber returns to your hand.");

        SEPARATOR_LINE
        BeatSaberUI::CreateText(floatGrid->get_transform(), "Technical numbers, please avoid.", false)->set_fontSize(sectTextMult);
        BeatSaberUI::AddHoverHint(AddConfigValueIncrementFloat(floatGrid->get_transform(), getPluginConfig().SlowmoStepAmount, 1, 0.1, 0, 10)->get_gameObject(),"The slow motion time scale amount.");
        BeatSaberUI::AddHoverHint(AddConfigValueIncrementFloat(floatGrid->get_transform(), getPluginConfig().SlowmoAmount, 2, 0.1, 0, 10)->get_gameObject(),"The intensity of the slow motion.");
        BeatSaberUI::AddHoverHint(AddConfigValueIncrementInt(floatGrid->get_transform(), getPluginConfig().VelocityBufferSize, 1, 0, 10)->get_gameObject(),"Technical number for the size of the list that holds velocity throughout time.");

        auto* actionGrid = container;
        SEPARATOR_LINE
        BeatSaberUI::CreateText(actionGrid->get_transform(), "Actions Remapping (UI is very funky here)", false)->set_fontSize(sectTextMult);
//        actionGrid->set_name(il2cpp_utils::newcsstr("Actions"));
//        actionGrid->set_spacing(1);

        BeatSaberUI::AddHoverHint(AddConfigValueDropdownEnum(actionGrid->get_transform(), getPluginConfig().TriggerAction, TrickAction, ACTION_NAMES, ACTION_REVERSE_NAMES)->get_gameObject(),"The action the trigger performs.");
        BeatSaberUI::AddHoverHint(AddConfigValueDropdownEnum(actionGrid->get_transform(), getPluginConfig().ButtonOneAction, TrickAction, ACTION_NAMES,ACTION_REVERSE_NAMES)->get_gameObject(),"The action the button one performs.");
        BeatSaberUI::AddHoverHint(AddConfigValueDropdownEnum(actionGrid->get_transform(), getPluginConfig().ButtonTwoAction, TrickAction, ACTION_NAMES, ACTION_REVERSE_NAMES)->get_gameObject(),"The action the button two performs.");
        BeatSaberUI::AddHoverHint(AddConfigValueDropdownEnum(actionGrid->get_transform(), getPluginConfig().GripAction, TrickAction, ACTION_NAMES, ACTION_REVERSE_NAMES)->get_gameObject(),"The action the grip button performs.");
        BeatSaberUI::AddHoverHint(AddConfigValueDropdownEnum(actionGrid->get_transform(), getPluginConfig().ThumbstickAction, TrickAction, ACTION_NAMES, ACTION_REVERSE_NAMES)->get_gameObject(),"The action the thumbstick performs.");

        SEPARATOR_LINE
        auto* miscGrid = container;
        BeatSaberUI::CreateText(miscGrid->get_transform(), "Misc", false)->set_fontSize(sectTextMult);
//        miscGrid->set_spacing(1);

        BeatSaberUI::AddHoverHint(AddConfigValueDropdownEnum(miscGrid->get_transform(), getPluginConfig().SpinDirection, TrickAction, SPIN_DIR_NAMES, SPIN_DIR_REVERSE_NAMES)->get_gameObject(),"The direction of spinning. Still dependent on reverse button");
        BeatSaberUI::AddHoverHint(AddConfigValueDropdownEnum(miscGrid->get_transform(), getPluginConfig().ThumbstickDirection, TrickAction, THUMBSTICK_DIR_NAMES, THUMBSTICK_DIR_REVERSE_NAMES)->get_gameObject(),"The direction of the thumbsticks for tricks. ");

    }
}

extern "C" void load() {
    il2cpp_functions::Init();
    // TODO: config menus
    getLogger().info("Installing hooks...");

    INSTALL_HOOK(getLogger(), SceneManager_Internal_SceneLoaded);
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
