#include "../include/TrickManager.hpp"
#include <algorithm>
#include <optional>
#include <queue>
#include "../include/AllInputHandlers.hpp"
#include "UnityEngine/Time.hpp"
#include "UnityEngine/Space.hpp"
#include "UnityEngine/AudioSource.hpp"
#include "PluginConfig.hpp"
#include "main.hpp"
#include <string>
#include <algorithm>
#include "UnityEngine/ScriptableObject.hpp"
#include "Libraries/HM/HMLib/VR/HapticPresetSO.hpp"
#include "beatsaber-hook/shared/utils/byref.hpp"

#include "UnityEngine/WaitForEndOfFrame.hpp"

using namespace Libraries::HM::HMLib::VR;
using namespace TrickSaber;
using namespace custom_types;
using namespace custom_types::Helpers;

// Define static fields
constexpr UnityEngine::Space RotateSpace = UnityEngine::Space::Self;


// for "ApplySlowmoSmooth" / "EndSlowmoSmooth"
static TrickState _slowmoState = Inactive;  // must also be reset in Start()
static float _slowmoTimeScale;
static float _originalTimeScale;
static float _targetTimeScale;
static UnityEngine::AudioSource* _audioSource;
static function_ptr_t<void, Il2CppObject*> RigidbodySleep;
static bool _gamePaused;

static SafePtrUnity<HapticPresetSO> hapticFeedbackThrowReturn;

static const MethodInfo* VRController_get_transform = nullptr;
static std::unordered_map<GlobalNamespace::VRController*, UnityEngine::Transform*> fakeTransforms;
static bool VRController_transform_is_hooked = false;
MAKE_HOOK_FIND(VRController_get_transform_hook, classof(GlobalNamespace::VRController*), "get_transform", UnityEngine::Transform*, GlobalNamespace::VRController* self) {


    if (!getPluginConfig().EnableTrickCutting.GetValue())
        return VRController_get_transform_hook(self);

    auto pair = fakeTransforms.find(self);
    if ( pair == fakeTransforms.end() ) {
        return VRController_get_transform_hook(self);
    } else {
//        getLogger().debug("Pair name: %s", to_utf8(csstrtostr(pair->second->get_name())).c_str());

        return pair->second;
    }
}

void ButtonMapping::Update() {
    // According to Oculus documentation, left is always Primary and Right is always secondary UNLESS referred to individually.
    // https://developer.oculus.com/reference/unity/v14/class_o_v_r_input
    GlobalNamespace::OVRInput::Controller oculusController;
    UnityEngine::XR::XRNode node;
    if (left) {
        oculusController = GlobalNamespace::OVRInput::Controller::LTouch;
        node = UnityEngine::XR::XRNode::LeftHand;
    } else {
        oculusController = GlobalNamespace::OVRInput::Controller::RTouch;
        node = UnityEngine::XR::XRNode::RightHand;
    }

    // Method missing from libil2cpp.so
    //auto* controllerInputDevice = CRASH_UNLESS(il2cpp_utils::RunMethod("UnityEngine.XR", "InputDevices", "GetDeviceAtXRNode", node));
    static auto getDeviceIdAtXRNode = (function_ptr_t<uint64_t, XRNode>)CRASH_UNLESS(
        il2cpp_functions::resolve_icall("UnityEngine.XR.InputTracking::GetDeviceIdAtXRNode"));
    getLogger().debug("getDeviceIdAtXRNode ptr offset: %lX", asOffset(getDeviceIdAtXRNode));
    auto deviceId = node; // getDeviceIdAtXRNode(node);
    auto controllerInputDevice = UnityEngine::XR::InputDevice(deviceId, false); // CRASH_UNLESS(il2cpp_utils::New("UnityEngine.XR", "InputDevice", deviceId));

    getLogger().debug("oculusController: %i", (int)oculusController);

    bool isOculus = GlobalNamespace::OVRInput::IsControllerConnected(oculusController);
    getLogger().debug("isOculus: %i", isOculus);

    auto dir = getPluginConfig().ThumbstickDirection.GetValue();

    actionHandlers.clear();
    actionHandlers[(int)getPluginConfig().TriggerAction.GetValue()].insert(std::unique_ptr<InputHandler>(
            new TriggerHandler(node, getPluginConfig().TriggerThreshold.GetValue())
    ));
    actionHandlers[(int)getPluginConfig().GripAction.GetValue()].insert(std::unique_ptr<InputHandler>(
            new GripHandler(isOculus, oculusController, controllerInputDevice, getPluginConfig().GripThreshold.GetValue())
    ));
    actionHandlers[(int)getPluginConfig().ThumbstickAction.GetValue()].insert(std::unique_ptr<InputHandler>(
            new ThumbstickHandler(node, getPluginConfig().ThumbstickThreshold.GetValue(), dir)
    ));
    actionHandlers[(int)getPluginConfig().ButtonOneAction.GetValue()].insert(std::unique_ptr<InputHandler>(
            new ButtonHandler(oculusController, GlobalNamespace::OVRInput::Button::One)
    ));
    actionHandlers[(int)getPluginConfig().ButtonTwoAction.GetValue()].insert(std::unique_ptr<InputHandler>(
            new ButtonHandler(oculusController, GlobalNamespace::OVRInput::Button::Two)
    ));
    if (actionHandlers[(int) TrickAction::Throw].empty()) {
        getLogger().warning("No inputs assigned to Throw! Throw will never trigger!");
    }
    if (actionHandlers[(int) TrickAction::Spin].empty()) {
        getLogger().warning("No inputs assigned to Spin! Spin will never trigger!");
    }
}


void TrickManager::LogEverything() {
    getLogger().debug("_throwState %i", _throwState);
    getLogger().debug("_spinState %i", _spinState);
    getLogger().debug("RotationSpeed: %f", _saberSpeed);
}

float getDeltaTime() {
    return RET_0_UNLESS(getLogger(), UnityEngine::Time::get_deltaTime());
}


// Copied from DnSpy
Sombrero::FastQuaternion Quaternion_Inverse(const Sombrero::FastQuaternion &q) {

//    Quaternion ret = CRASH_UNLESS(il2cpp_utils::RunMethod<Quaternion>(cQuaternion, "Inverse", q));
    return Sombrero::FastQuaternion::Inverse(q);
}

Sombrero::FastVector3 GetAngularVelocity(const Sombrero::FastQuaternion& foreLastFrameRotation, const Sombrero::FastQuaternion& lastFrameRotation)
{
    auto foreLastInv = Quaternion_Inverse(foreLastFrameRotation);
    auto q = lastFrameRotation * foreLastInv;
    if (abs(q.w) > (1023.5f / 1024.0f)) {
        return Vector3_Zero;
    }
    float gain;
    if (q.w < 0.0f) {
        auto angle = acos(-q.w);
        gain = (float) (-2.0f * angle / (sin(angle) * getDeltaTime()));
    } else {
        auto angle = acos(q.w);
        gain = (float) (2.0f * angle / (sin(angle) * getDeltaTime()));
    }

    return {q.x * gain, q.y * gain, q.z * gain};
}

void TrickManager::AddProbe(const Sombrero::FastVector3& vel, const Sombrero::FastVector3& ang) {
    if (_currentProbeIndex >= _velocityBuffer.size()) _currentProbeIndex = 0;
    _velocityBuffer[_currentProbeIndex] = vel;
    _angularVelocityBuffer[_currentProbeIndex] = ang;
    _currentProbeIndex++;
}

Sombrero::FastVector3 TrickManager::GetAverageVelocity() {
    Sombrero::FastVector3 avg = Vector3_Zero;
    for (auto & i : _velocityBuffer) {
        avg += i;
    }
    return avg / (float) _velocityBuffer.size();
}

Sombrero::FastVector3 TrickManager::GetAverageAngularVelocity() {
    Sombrero::FastVector3 avg = Vector3_Zero;
    for (int i = 0; i < _velocityBuffer.size(); i++) {
        avg += _angularVelocityBuffer[i];
    }
    return avg / (float) _velocityBuffer.size();
}

UnityEngine::Transform* TrickManager::FindBasicSaberTransform() {
    auto* basicSaberT = _saberT->Find(_basicSaberName);
    if (!basicSaberT) {
        auto* saberModelT = _saberT->Find(_saberName);
        basicSaberT = saberModelT->Find(_basicSaberName);
    }
    return CRASH_UNLESS(basicSaberT);
}

void TrickManager::Start2() {
    getLogger().debug("TrickManager.Start2!");
    UnityEngine::Transform* saberModelT;
    UnityEngine::Transform* basicSaberT = nullptr;
    if (!getPluginConfig().EnableTrickCutting.GetValue()) {
        // Try to find a custom saber - will have same name as _saberT (LeftSaber or RightSaber) but be a child of it

        saberModelT = _saberT->Find(_saberName);
        if(!saberModelT) saberModelT = nullptr;

        basicSaberT = FindBasicSaberTransform();

        if (!saberModelT) {
            getLogger().warning("Did not find custom saber! Thrown sabers will be BasicSaberModel(Clone)!");
            saberModelT = basicSaberT;
        } else if (getenv("qsabersenabled")) {
            getLogger().debug("Not moving trail because Qosmetics is installed!");
        }
    } else {
        saberModelT = Saber->get_transform();
    }
    CRASH_UNLESS(saberModelT);
    auto* saberGO = saberModelT->get_gameObject();
    getLogger().debug("root Saber gameObject: %p", Saber->get_gameObject());
    _saberTrickModel = new SaberTrickModel(Saber, saberGO, saberModelT == basicSaberT || getPluginConfig().EnableTrickCutting.GetValue());
    // note that this is the transform of the whole Saber (as opposed to just the model) iff TrickCutting
    _originalSaberModelT = saberGO->get_transform();

    if (!hapticFeedbackThrowReturn) {
        hapticFeedbackThrowReturn.emplace(UnityEngine::ScriptableObject::CreateInstance<HapticPresetSO *>());
        hapticFeedbackThrowReturn->duration = 0.15f;
        hapticFeedbackThrowReturn->strength = 1.2f;
        hapticFeedbackThrowReturn->frequency = 0.3f;
    }
}

void TrickManager::StaticClear() {
    _slowmoState = Inactive;
    audioTimeSyncController = nullptr;
    _audioSource = nullptr;
    _gamePaused = false;
}

void TrickManager::Clear() {
    setSpinState(Inactive);
    setThrowState(Inactive);
    delete _saberTrickModel;
    _saberTrickModel = nullptr;
    _originalSaberModelT = nullptr;
}

void TrickManager::Start() {
    getLogger().debug("Audio");
    if (!audioTimeSyncController) {
        getLogger().debug("Audio controllers: %p", audioTimeSyncController);
        // TODO: Is necessary?
//        CRASH_UNLESS(audioTimeSyncController);
    }
    getLogger().debug("Rigid body sleep clash");
    if (!RigidbodySleep) {
        RigidbodySleep = (decltype(RigidbodySleep))CRASH_UNLESS(il2cpp_functions::resolve_icall("UnityEngine.Rigidbody::Sleep"));
        getLogger().debug("RigidbodySleep ptr offset: %lX", asOffset(RigidbodySleep));
    }

    // auto* rigidbody = CRASH_UNLESS(GetComponent(Saber, "UnityEngine", "Rigidbody"));

    if (Saber->GetComponents<UnityEngine::BoxCollider *>().Length() > 0)
        _collider = Saber->GetComponent<UnityEngine::BoxCollider *>();

    if (VRController)
        _vrPlatformHelper = VRController->vrPlatformHelper;

    _buttonMapping = ButtonMapping(_isLeftSaber);

    int velBufferSize = getPluginConfig().VelocityBufferSize.GetValue();
    _velocityBuffer = std::vector<Sombrero::FastVector3>(velBufferSize);
    _angularVelocityBuffer = std::vector<Sombrero::FastVector3>(velBufferSize);

    _saberT = Saber->get_transform();
    _saberName = Saber->get_name();
    getLogger().debug("saberName: %s", to_utf8(csstrtostr(_saberName)).c_str());
    _basicSaberName = il2cpp_utils::newcsstr("BasicSaberModel(Clone)");

    getLogger().debug("Setting states to inactive due to start");
    setThrowState(Inactive);
    setSpinState(Inactive);

    if (getPluginConfig().EnableTrickCutting.GetValue()) {
        if (!VRController_get_transform) {
            getLogger().debug("VRC");
            VRController_get_transform = CRASH_UNLESS(il2cpp_utils::FindMethod("", "VRController", "get_transform"));
            getLogger().debug("VRCCCCCCCC");
        }
        auto* fakeTransformName = CRASH_UNLESS(il2cpp_utils::newcsstr("FakeTrickSaberTransform"));
        auto* fakeTransformGO = UnityEngine::GameObject::New_ctor(fakeTransformName); // CRASH_UNLESS(il2cpp_utils::New("UnityEngine", "GameObject", fakeTransformName));
        _fakeTransform = fakeTransformGO->get_transform();

        auto* saberParentT = _saberT->get_parent();
        _fakeTransform->SetParent(saberParentT);
        auto fakePos = _fakeTransform->get_localPosition();
        getLogger().debug("fakePos: {%f, %f, %f} parent: %s", fakePos.x, fakePos.y, fakePos.z, to_utf8(csstrtostr(saberParentT->get_name())).c_str());

        // TODO: instead of patching this transform onto the VRController, add a clone VRController component to the object?
    }
    getLogger().debug("Leaving TrickManager.Start");
}

static float GetTimescale() {
    auto& audioTimeSyncController = TrickManager::audioTimeSyncController;

//    if (!audioTimeSyncController) return _slowmoTimeScale;

    return audioTimeSyncController->get_timeScale();
}

void SetTimescale(float timescale) {
    auto& audioTimeSyncController = TrickManager::audioTimeSyncController;
    // Efficiency is top priority in FixedUpdate!
    if (audioTimeSyncController) {
        audioTimeSyncController->timeScale = timescale;
        auto songTime = audioTimeSyncController->songTime;
        audioTimeSyncController->startSongTime = songTime;
        auto timeSinceStart = audioTimeSyncController->get_timeSinceStart();
        auto songTimeOffset = audioTimeSyncController->songTimeOffset;
        audioTimeSyncController->audioStartTimeOffsetSinceStart = timeSinceStart - (songTime + songTimeOffset);
        audioTimeSyncController->fixingAudioSyncError = false;
        audioTimeSyncController->playbackLoopIndex = 0;

        if (audioTimeSyncController->get_isAudioLoaded()) return;

        if (_audioSource) _audioSource->set_pitch(timescale);
    }
}

void ForceEndSlowmo() {
    if (_slowmoState != Inactive) {
        SetTimescale(_targetTimeScale);
        _slowmoState = Inactive;
        getLogger().debug("Slowmo ended. TimeScale: %f", _targetTimeScale);
    }
}

void TrickManager::StaticFixedUpdate() {
    // Efficiency is top priority in FixedUpdate!
    if (_gamePaused) return;
    if (_slowmoState == Started) {
        // IEnumerator ApplySlowmoSmooth
        if (_slowmoTimeScale > _targetTimeScale) {
            _slowmoTimeScale -= getPluginConfig().SlowmoStepAmount.GetValue();
            SetTimescale(_slowmoTimeScale);
        } else if (_slowmoTimeScale != _targetTimeScale) {
            getLogger().debug("Slowmo == Started; Forcing TimeScale from %f to %f", _slowmoTimeScale, _targetTimeScale);
            _slowmoTimeScale = _targetTimeScale;
            SetTimescale(_slowmoTimeScale);
        }
    } else if (_slowmoState == Ending) {
        // IEnumerator EndSlowmoSmooth
        if (_slowmoTimeScale < _targetTimeScale) {
            _slowmoTimeScale += getPluginConfig().SlowmoStepAmount.GetValue();
            SetTimescale(_slowmoTimeScale);
        } else if (_slowmoTimeScale != _targetTimeScale) {
            getLogger().debug("Slowmo == Ending; Forcing TimeScale from %f to %f", _slowmoTimeScale, _targetTimeScale);
            _slowmoTimeScale = _targetTimeScale;
            SetTimescale(_slowmoTimeScale);
        }
    }
}

void TrickManager::FixedUpdate() {
    // Efficiency is top priority in FixedUpdate!
    if (!_saberTrickModel) return;
}

void TrickManager::Update() {
    if (!_saberTrickModel) {
        _timeSinceStart += getDeltaTime();
        if (getPluginConfig().EnableTrickCutting.GetValue() || _saberT->Find(_saberName) ||
            _timeSinceStart > 1) {
            Start2();
        } else {
            return;
        }
    }
    if (_gamePaused) return;

    if (VRController == nullptr || !VRController->get_enabled())
        return;

    if (getPluginConfig().EnableTrickCutting.GetValue() && ((_spinState != Inactive) || (_throwState != Inactive))) {
        VRController->Update(); // sets position and pre-_currentRotation
    }

    // Note: if TrickCutting, during throw, these properties are redirected to an unused object
    _controllerPosition = VRController->get_position();
    _controllerRotation = VRController->get_rotation();

    auto dPos = _controllerPosition - _prevPos;
    auto velocity = dPos / getDeltaTime();
    _angularVelocity = GetAngularVelocity(_prevRot, _controllerRotation);

    if (_fakeTransform) {
        auto fakePos = _fakeTransform->get_localPosition();
//        getLogger().debug("fakePos: {%f, %f, %f} update %s", fakePos.x, fakePos.y, fakePos.z,
//                          to_utf8(csstrtostr(VRController->get_transform()->get_name())).c_str());
    }

    auto dCon = _prevPos - _controllerPosition;

    if (getPluginConfig().MoveWhileThrown.GetValue() && _throwState != TrickState::Inactive) {
        auto* rigidBody = _saberTrickModel->Rigidbody;

        static auto addTorque = (function_ptr_t<void, UnityEngine::Rigidbody*, UnityEngine::Vector3*, int>)
                CRASH_UNLESS(il2cpp_functions::resolve_icall("UnityEngine.Rigidbody::AddTorque_Injected"));

        auto torqueVel = _angularVelocity;
        addTorque(rigidBody, &torqueVel, 0);
    }

    float distanceController = dCon.Magnitude();

    // float mag = Vector3_Magnitude(_angularVelocity);
    // if (mag) getLogger().debug("angularVelocity.x: %f, .y: %f, mag: %f", _angularVelocity.x, _angularVelocity.y, mag);
    AddProbe(velocity, _angularVelocity);
    _saberSpeed = velocity.Magnitude();
    _prevPos = _controllerPosition;
    _prevRot = _controllerRotation;

    // TODO: move these to LateUpdate?
    if (_throwState == Ending) {
        Sombrero::FastVector3 saberPos = _saberTrickModel->Rigidbody->get_position();

        Sombrero::FastVector3 saberLogPos = _saberTrickModel->Rigidbody->get_transform()->get_localPosition();

//        getLogger().debug("Saber (%f %f %f) and controller pos (%f %f %f) and dist %f", saberLogPos.x, saberLogPos.y, saberLogPos.z, _controllerPosition.x, _controllerPosition.y, _controllerPosition.z, distanceController);

        auto d = _controllerPosition - saberPos;
        float distance = d.Magnitude();

        if (distance <= getPluginConfig().ControllerSnapThreshold.GetValue()) {
            ThrowEnd();
        } else {
            float returnSpeed = fmax(distance, 1.0f) * getPluginConfig().ReturnSpeed.GetValue();
//            getLogger().debug("distance: %f; return speed: %f", distance, returnSpeed);
            auto dirNorm = d.get_normalized();
            auto newVel = dirNorm * returnSpeed;

            _saberTrickModel->Rigidbody->set_velocity(newVel);
        }
    }
    // TODO: no tricks while paused? https://github.com/ToniMacaroni/TrickSaber/blob/ea60dce35db100743e7ba72a1ffbd24d1472f1aa/TrickSaber/SaberTrickManager.cs#L66


    if (audioTimeSyncController != nullptr)
        CheckButtons();
}

ValueTuple TrickManager::GetTrackingPos() {
    UnityEngine::XR::XRNode controllerNode = VRController->get_node();
    int controllerNodeIdx = VRController->get_nodeIdx();

    ValueTuple result{};

    bool nodePose = _vrPlatformHelper->GetNodePose(controllerNode,controllerNodeIdx, result.item1, result.item2); // CRASH_UNLESS(il2cpp_utils::RunMethod<bool>(_vrPlatformHelper, "GetNodePose", controllerNode,
        //controllerNodeIdx, result.item1, result.item2));
    if (!nodePose) {
        getLogger().warning("Node pose missing for %s controller!", _isLeftSaber ? "Left": "Right");
        result.item1 = {-0.2f, 0.05f, 0.0f};
        result.item2 = {0.0f, 0.0f, 0.0f, 1.0f};
    }
    return result;
}

bool CheckHandlersDown(decltype(ButtonMapping::actionHandlers)::mapped_type& handlers, float& power) {
    power = 0;
    // getLogger().debug("handlers.size(): %lu", handlers.size());
    if (handlers.empty()) return false;
    for (auto& handler : handlers) {
        float val;
        if (!handler->Activated(val)) {
            return false;
        }
        power += val;
    }
    power /= handlers.size();
    return true; 
}

bool CheckHandlersUp(decltype(ButtonMapping::actionHandlers)::mapped_type& handlers) {
    for (auto& handler : handlers) {
        if (handler->Deactivated()) return true;
    }
    return false;
}

static std::string actionToString(TrickState state) {
    switch (state) {
        case Inactive:
            return "INACTIVE";
        case Started:
            return "STARTED";
        case Ending:
            return "ENDING";
    }
}

void TrickManager::CheckButtons() {

    bool tricksEnabled = getPluginConfig().TricksEnabled.GetValue();

    if (!tricksEnabled) {
        if (_throwState == Started) {
            ThrowReturn();
        }

        if (_spinState == Started) {
            InPlaceRotationReturn();
        }
        doFrozenThrow = false;

        return;
    }

    // Disable tricks while viewing replays.
    auto* replayMode = getenv("ViewingReplay");

//    getLogger().debug("replay mode val: %s", replayMode);

    // TODO: Remove false condition here when replay fixes bug
    if (replayMode && strcmp(replayMode, "true") == 0) return;

    float power;

    auto freezeThrowDown = CheckHandlersDown(_buttonMapping.actionHandlers[(int) TrickAction::FreezeThrow], power);
    auto freezeThrowUp = CheckHandlersUp(_buttonMapping.actionHandlers[(int) TrickAction::FreezeThrow]);
    auto throwDown = CheckHandlersDown(_buttonMapping.actionHandlers[(int) TrickAction::Throw], power);
    auto throwUp = CheckHandlersUp(_buttonMapping.actionHandlers[(int) TrickAction::Throw]);
    auto spinDown = CheckHandlersDown(_buttonMapping.actionHandlers[(int) TrickAction::Spin], power);
    auto spinUp = CheckHandlersUp(_buttonMapping.actionHandlers[(int) TrickAction::Spin]);

    if (!objectDestroyTimes.empty()) {
//        std::vector<int64_t> copyObjectDestroyTimes = objectDestroyTimes;
//        getLogger().debug("Object destroy");
        auto it = objectDestroyTimes.begin();
        while (it != objectDestroyTimes.end()) {
            auto destroyTime = *it;

            if (!destroyTime)
                continue;

            if (getTimeMillis() - destroyTime > 700) {
                objectCount--;
                if (objectCount < 0) objectCount = 0;
                it = objectDestroyTimes.erase(it);
            } else {
                it++;
            }
        }
    }

    // Make the sabers return to original state.
    if (getPluginConfig().NoTricksWhileNotes.GetValue() && objectCount > 0) {
        throwDown = false;
        throwUp = true;
        spinDown = false;
        spinUp = true;
    }

    if ((_throwState != Ending) && throwDown) {
        ThrowStart();
    } else if ((_throwState == Started) && throwUp) {
        ThrowReturn();
    } else if ((_spinState != Ending) && spinDown) {
        InPlaceRotation(power);
    } else if ((_spinState == Started) && spinUp) {
        InPlaceRotationReturn();
    }

    if (freezeThrowDown) {
        doFrozenThrow = true;
    } else if (freezeThrowUp) {
        doFrozenThrow = false;
    }
}


void TrickManager::TrickStart() const {
    if (getPluginConfig().EnableTrickCutting.GetValue()) {
        // even on throws, we disable this to call Update manually and thus control ordering
        VRController->set_enabled(false);
    } else {
        DisableBurnMarks(_isLeftSaber ? 0 : 1);
//        this->doClashEffect = false;
    }
}

void TrickManager::TrickEnd() const {
    if (getPluginConfig().EnableTrickCutting.GetValue()) {
        VRController->set_enabled(true);
    } else {
        if ((other->_throwState == Inactive) && (other->_spinState == Inactive)) {
            //        this->doClashEffect = true;
        }
        EnableBurnMarks(_isLeftSaber ? 0 : 1);
    }
}


void ListActiveChildren(Il2CppObject* root, std::string_view name) {
    auto* rootT = CRASH_UNLESS(il2cpp_utils::GetPropertyValue(root, "transform"));
    std::queue<Il2CppObject*> frontier;
    frontier.push(rootT);
    while (!frontier.empty()) {
        auto* transform = frontier.front();
        auto* go = CRASH_UNLESS(il2cpp_utils::GetPropertyValue(transform, "gameObject"));
        auto* goName = CRASH_UNLESS(il2cpp_utils::GetPropertyValue<Il2CppString*>(go, "name"));
        std::string parentName = to_utf8(csstrtostr(goName));
        frontier.pop();
        int children = CRASH_UNLESS(il2cpp_utils::GetPropertyValue<int>(transform, "childCount"));
        for (int i = 0; i < children; i++) {
            auto* childT = CRASH_UNLESS(il2cpp_utils::RunMethod(transform, "GetChild", i));
            if (childT) {
                auto* child = CRASH_UNLESS(il2cpp_utils::GetPropertyValue(childT, "gameObject"));
                if (CRASH_UNLESS(il2cpp_utils::GetPropertyValue<bool>(child, "activeInHierarchy"))) {
                    goName = CRASH_UNLESS(il2cpp_utils::GetPropertyValue<Il2CppString*>(child, "name"));
                    std::string childName = to_utf8(csstrtostr(goName));
                    getLogger().debug("found %s child '%s'", name.data(), (parentName + "/" + childName).c_str());
                }
                frontier.push(childT);
            }
        }
    }
}

void TrickManager::EndTricks() {
    ThrowReturn();
    InPlaceRotationReturn();
}

static void CheckAndLogAudioSync() {
    // TODO: Figure out why this is here.

    return;
    auto& audioTimeSyncController = TrickManager::audioTimeSyncController;

    if (audioTimeSyncController->fixingAudioSyncError)
        getLogger().warning("audioTimeSyncController is fixing audio time sync issue!");
    auto timeSinceStart = audioTimeSyncController->get_timeSinceStart();
    auto audioStartTimeOffsetSinceStart = audioTimeSyncController->audioStartTimeOffsetSinceStart;
    if (timeSinceStart < audioStartTimeOffsetSinceStart) {
        getLogger().warning("timeSinceStart < audioStartTimeOffsetSinceStart! _songTime may progress while paused! %f, %f",
            timeSinceStart, audioStartTimeOffsetSinceStart);
    } else {
        getLogger().debug("timeSinceStart, audioStartTimeOffsetSinceStart: %f, %f", timeSinceStart, audioStartTimeOffsetSinceStart);
    }
    getLogger().debug("_songTime: %f", audioTimeSyncController->songTime);
}

void TrickManager::StaticPause() {
    _gamePaused = true;
    getLogger().debug("Paused with TimeScale %f", _slowmoTimeScale);
    CheckAndLogAudioSync();
}

void TrickManager::PauseTricks() {
    if (_saberTrickModel)
        _saberTrickModel->SaberGO->SetActive(false);
}

void TrickManager::StaticResume() {
    _gamePaused = false;
    _slowmoTimeScale = GetTimescale();
//    getLogger().debug("Resumed with TimeScale %f", _slowmoTimeScale);
//    CheckAndLogAudioSync();
}

void TrickManager::ResumeTricks() {
    if (_saberTrickModel)
        _saberTrickModel->SaberGO->SetActive(true);
}

void TrickManager::ThrowStart() {
    if (_throwState == Inactive) {
        getLogger().debug("%s throw start!", _isLeftSaber ? "Left" : "Right");

        if (!getPluginConfig().EnableTrickCutting.GetValue()) {
            _saberTrickModel->ChangeToTrickModel();
            // ListActiveChildren(Saber, "Saber");
            // ListActiveChildren(_saberTrickModel->SaberGO, "custom saber");
        } else {
            if (_collider->get_enabled())
                _collider->set_enabled(false);
        }
        TrickStart();

        auto* rigidBody = _saberTrickModel->Rigidbody;
        rigidBody->set_isKinematic(false);

        Sombrero::FastVector3 velo = GetAverageVelocity();
        float _velocityMultiplier = getPluginConfig().ThrowVelocity.GetValue();
        velo = velo * 3 * _velocityMultiplier;
        rigidBody->set_velocity(velo);

        _saberRotSpeed = _saberSpeed * _velocityMultiplier;
        // getLogger().debug("initial _saberRotSpeed: %f", _saberRotSpeed);
        if (_angularVelocity.x > 0) {
            _saberRotSpeed *= 150;
        } else {
            _saberRotSpeed *= -150;
        }


        auto* saberTransform = rigidBody->get_transform();
        getLogger().debug("velocity: %f", velo.Magnitude());
        getLogger().debug("_saberRotSpeed: %f", _saberRotSpeed);
        auto torqRel = Vector3_Right * _saberRotSpeed;
        auto torqWorld = saberTransform->TransformVector(torqRel);
        // 5 == ForceMode.Acceleration

        // Cannot use Codegen here since it is stripped
        getLogger().debug("Adding torque");
        static auto addTorque = (function_ptr_t<void, UnityEngine::Rigidbody*, UnityEngine::Vector3*, int>)
                CRASH_UNLESS(il2cpp_functions::resolve_icall("UnityEngine.Rigidbody::AddTorque_Injected"));

        getLogger().debug("Torque method torque: %s", addTorque == nullptr ? "true" : "false");

        addTorque(rigidBody, &torqWorld, 0);
        getLogger().debug("Added torque");

        if (getPluginConfig().EnableTrickCutting.GetValue()) {
            fakeTransforms.emplace(VRController, _fakeTransform);
            if (!VRController_transform_is_hooked) {
                INSTALL_HOOK(getLogger(), VRController_get_transform_hook);
                VRController_transform_is_hooked = true;
            }
        }

//        DisableBurnMarks(_isLeftSaber ? 0 : 1);

        getLogger().debug("Throw state set");
        setThrowState(Started);

        if (getPluginConfig().SlowmoDuringThrow.GetValue()) {
            if (!audioTimeSyncController) {
                getLogger().debug("No audio time sync controller?");
                return;
            }
            _audioSource = audioTimeSyncController->audioSource;
            if (_slowmoState != Started) {
                // ApplySlowmoSmooth
                _slowmoTimeScale = GetTimescale();
                _originalTimeScale = (_slowmoState == Inactive) ? _slowmoTimeScale : _targetTimeScale;

                _targetTimeScale = _originalTimeScale - getPluginConfig().SlowmoAmount.GetValue();
                if (_targetTimeScale < 0.1f) _targetTimeScale = 0.1f;

                getLogger().debug("Starting slowmo; TimeScale from %f (%f original) to %f", _slowmoTimeScale, _originalTimeScale, _targetTimeScale);
                _slowmoState = Started;
            }
        }
    } else {
        auto* rigidBody = _saberTrickModel->Rigidbody;
        if (doFrozenThrow) {
            rigidBody->set_velocity(0);
        }
        getLogger().debug("%s throw continues", _isLeftSaber ? "Left" : "Right");
    }

    // ThrowUpdate();  // not needed as long as the velocity and torque do their job
}

void TrickManager::ThrowUpdate() {
    // auto* saberTransform = CRASH_UNLESS(il2cpp_utils::GetPropertyValue(_saberTrickModel->SaberGO, "transform"));
    // // // For when AddTorque doesn't work
    // // CRASH_UNLESS(il2cpp_utils::RunMethod(saberTransform, "Rotate", Vector3_Right, _saberRotSpeed, RotateSpace));

    // auto rot = CRASH_UNLESS(il2cpp_utils::GetPropertyValue<Quaternion>(saberTransform, "rotation"));
    // auto angVel = GetAngularVelocity(_prevTrickRot, rot);
    // getLogger().debug("angVel.x: %f, .y: %f, mag: %f", angVel.x, angVel.y, Vector3_Magnitude(angVel));
    // _prevTrickRot = rot;
}

void TrickManager::ThrowReturn() {
    if (_throwState == Started) {
        getLogger().debug("%s throw return!", _isLeftSaber ? "Left" : "Right");

        _saberTrickModel->Rigidbody->set_velocity(Vector3_Zero);
        setThrowState(Ending);

        Sombrero::FastVector3 saberPos = _saberTrickModel->Rigidbody->get_position();
        _throwReturnDirection = _controllerPosition - saberPos;
        getLogger().debug("distance: %f", _throwReturnDirection.Magnitude());

        if ((_slowmoState == Started) && (other->_throwState != Started)) {
            _slowmoTimeScale = GetTimescale();
            _targetTimeScale = _originalTimeScale;
            _audioSource = audioTimeSyncController->audioSource;
            getLogger().debug("Ending slowmo; TimeScale from %f to %f", _slowmoTimeScale, _targetTimeScale);
            _slowmoState = Ending;
        }
    }
}

void TrickManager::ThrowEnd() {
    getLogger().debug("%s throw end!", _isLeftSaber ? "Left" : "Right");
    _saberTrickModel->Rigidbody->set_isKinematic(true);  // restore
    if (!getPluginConfig().EnableTrickCutting.GetValue()) {
        _saberTrickModel->ChangeToActualSaber();
    } else {
        _collider->set_enabled(true);

//        _saberT->set_position(Vector3_Zero);
//        _saberT->set_rotation(Quaternion_Identity);
//        _saberT->set_localRotation(Quaternion_Identity);
        fakeTransforms.erase(VRController);

        // Update the transform
        VRController->Update();
//
//
//        _saberT->get_parent()->set_localPosition(Vector3_Zero);
//        _saberT->get_parent()->set_position(VRController->get_position());
////
////        VRController->get_transform()->set_localPosition(Vector3_Zero);
//        _saberTrickModel->Rigidbody->get_transform()->set_localPosition(Vector3_Zero);
//        _saberTrickModel->SaberGO->get_transform()->set_localPosition(Vector3_Zero);
//        _saberTrickModel->SaberGO->get_transform()->set_position(Vector3_Zero);
////        _saberTrickModel->Rigidbody->get_transform()->set_position(VRController->get_position());
////        _saberTrickModel->Rigidbody->get_transform()->set_rotation(VRController->get_rotation());
////        _saberTrickModel->Rigidbody->get_transform()->set_localRotation(Quaternion_Identity);
//        _saberT->set_localPosition(Vector3_Zero);
//        Saber->saberBladeBottomTransform->set_localPosition(Vector3_Zero);
//
////        _saberT->set_position(VRController->get_position());
//
////        _saberT->set_rotation(VRController->get_rotation());
////        _saberT->set_localRotation(Quaternion_Identity);
//
////        _originalSaberModelT->set_rotation(VRController->get_rotation());
//        _originalSaberModelT->set_localRotation(Quaternion_Identity);
//        _originalSaberModelT->set_localPosition(Vector3_Zero);
////        _originalSaberModelT->set_position(VRController->get_position());
    }
    if (other->_throwState == Inactive) {
        ForceEndSlowmo();
    }
    setThrowState(Inactive);

    UnityEngine::XR::XRNode node;
    if (_isLeftSaber) {
        node = UnityEngine::XR::XRNode::LeftHand;
    } else {
        node = UnityEngine::XR::XRNode::RightHand;
    }

    // Trigger vibration when saber returns
    if (getPluginConfig().VibrateOnReturn.GetValue())
        _hapticFeedbackController->PlayHapticFeedback(node, (HapticPresetSO *) hapticFeedbackThrowReturn);
//        _vrPlatformHelper->TriggerHapticPulse(node, 2, 2.2f, 0.3f);

    TrickEnd();
}


void TrickManager::InPlaceRotationStart() {
    getLogger().debug("%s rotate start!", _isLeftSaber ? "Left" : "Right");
    TrickStart();
    if (getPluginConfig().EnableTrickCutting.GetValue()) {
        _currentRotation = 0.0f;
    }

    if (getPluginConfig().IsVelocityDependent.GetValue()) {
        auto angularVelocity = GetAverageAngularVelocity();
        _spinSpeed = abs(angularVelocity.x) + abs(angularVelocity.y);
        // getLogger().debug("_spinSpeed: %f", _spinSpeed);
        auto contRotInv = Quaternion_Inverse(_controllerRotation);
        angularVelocity = contRotInv * angularVelocity;
        if (angularVelocity.x < 0) _spinSpeed *= -1;
    } else {
        float speed = 30;
        if (getPluginConfig().SpinDirection.GetValue() == (int) SpinDir::Backward) speed *= -1;
        _spinSpeed = speed;
    }
    _spinSpeed *= getPluginConfig().SpinSpeed.GetValue();
    setSpinState(Started);
}

void TrickManager::InPlaceRotationReturn() {
    if (_spinState == Started) {
        getLogger().debug("%s spin return!", _isLeftSaber ? "Left" : "Right");
        setSpinState(Ending);
        // where the PC mod would start a coroutine here, we'll wind the spin down starting in next TrickManager::Update
        // so just to maintain the movement: (+ restore the rotation that was reset by VRController.Update iff TrickCutting)

        // Fern scratch that, finally got coroutines, thanks scad!
        auto coro = getPluginConfig().CompleteRotationMode.GetValue() ? CoroutineHelper::New(CompleteRotation()) : CoroutineHelper::New(LerpToOriginalRotation());
        _saberTrickModel->saberScript->StartCoroutine(coro);
    }
}

Coroutine TrickManager::CompleteRotation() {
    auto minSpeed = 8.0f;
    auto largestSpinSpeed = _finalSpinSpeed;

    if (std::abs(largestSpinSpeed) < minSpeed)
    {
        largestSpinSpeed = _finalSpinSpeed < 0 ? -minSpeed : minSpeed;
    }



    auto threshold = std::abs(_finalSpinSpeed) + 0.1f;
    auto angle = Sombrero::FastQuaternion::Angle(_originalSaberModelT->get_localRotation(), Quaternion_Identity);

    while (angle > threshold)
    {
        _originalSaberModelT->Rotate(Sombrero::FastVector3::get_right() * _finalSpinSpeed);
        angle = Sombrero::FastQuaternion::Angle(_originalSaberModelT->get_localRotation(), Quaternion_Identity);
        co_yield reinterpret_cast<enumeratorT>(UnityEngine::WaitForEndOfFrame::New_ctor());
    }

    _originalSaberModelT->set_localRotation(Quaternion_Identity);
    setSpinState(Inactive);
    TrickEnd();
}

Coroutine TrickManager::LerpToOriginalRotation() {
    auto rot = _originalSaberModelT->get_localRotation();
    while (Sombrero::FastQuaternion::Angle(rot, Quaternion_Identity) > 5.0f)
    {
        rot = Sombrero::FastQuaternion::Lerp(rot, Quaternion_Identity, UnityEngine::Time::get_deltaTime() * 20);
        _originalSaberModelT->set_localRotation(rot);
        co_yield reinterpret_cast<enumeratorT>(UnityEngine::WaitForEndOfFrame::New_ctor());
    }

    _originalSaberModelT->set_localRotation(Quaternion_Identity);

    setSpinState(Inactive);
    TrickEnd();
}

void TrickManager::InPlaceRotationEnd() {
    if (!getPluginConfig().EnableTrickCutting.GetValue()) {
        _originalSaberModelT->set_localRotation(Quaternion_Identity);
    } else {
        _originalSaberModelT->set_localRotation(_controllerRotation);
    }

    getLogger().debug("%s spin end!", _isLeftSaber ? "Left" : "Right");
}



void TrickManager::_InPlaceRotate(float amount) {
    if (!getPluginConfig().EnableTrickCutting.GetValue()) {
        _originalSaberModelT->Rotate(Vector3_Right, amount, RotateSpace);
    } else {
        _currentRotation += amount;
        _originalSaberModelT->Rotate(Vector3_Right, _currentRotation, RotateSpace);
    }
}

void TrickManager::InPlaceRotation(float power) {
    if (_spinState == Inactive) {
        InPlaceRotationStart();
    } else {
        getLogger().debug("%s rotation continues! power %f", _isLeftSaber ? "Left" : "Right", power);
    }

    if (getPluginConfig().IsVelocityDependent.GetValue()) {
        _finalSpinSpeed = _spinSpeed;
    } else {
        _finalSpinSpeed = _spinSpeed * pow(power, 3.0f);  // power is the degree to which the held buttons are pressed
    }
    _InPlaceRotate(_finalSpinSpeed);
}

void TrickManager::setThrowState(TrickState state) {
    _throwState = state;
}

void TrickManager::setSpinState(TrickState state) {
    _spinState = state;
}

bool TrickManager::isDoingTricks() const {
    return _spinState != Inactive || _throwState != Inactive;
}

TrickSaber::TrickState TrickManager::getSpinState() const {
    return _spinState;
}

TrickSaber::TrickState TrickManager::getThrowState() const {
    return _throwState;
}

UnityEngine::GameObject * TrickManager::getNormalSaber() const {
    if (!_saberTrickModel)
        return nullptr;

    return _saberTrickModel->getOriginalModel();
}

UnityEngine::GameObject * TrickManager::getActiveSaber() const {
    if (!_saberTrickModel)
        return nullptr;

    return _saberTrickModel->getActiveModel();
}

UnityEngine::GameObject * TrickManager::getTrickSaber() const {
    if (!_saberTrickModel)
        return nullptr;

    return _saberTrickModel->getTrickModel();
}

SaberTrickModel *TrickManager::getTrickModel() const {
    return _saberTrickModel;
}
