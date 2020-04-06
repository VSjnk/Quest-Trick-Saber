#include "../include/TrickManager.hpp"
// Define static fields
// According to Oculus documentation, left is always Primary and Right is always secondary UNLESS referred to individually.
// https://developer.oculus.com/reference/unity/v14/class_o_v_r_input
ButtonMapping ButtonMapping::LeftButtons = ButtonMapping(PrimaryIndexTrigger, PrimaryThumbstickLeft);
ButtonMapping ButtonMapping::RightButtons = ButtonMapping(SecondaryIndexTrigger, SecondaryThumbstickRight);
Space RotateSpace = Self;

Il2CppClass *OVRInput = nullptr;
Controller ControllerMask;


void TrickManager::LogEverything() {
    log(DEBUG, "_isThrowing %i", _isThrowing);
    log(DEBUG, "_isRotatingInPlace %i", _isRotatingInPlace);
    log(DEBUG, "RotationSpeed: %f", _saberSpeed);
}

float getDeltaTime() {
    return *RET_0_UNLESS(il2cpp_utils::RunMethod<float>("UnityEngine", "Time", "get_deltaTime"));
}


Vector3 Vector3_Zero = {0.0f, 0.0f, 0.0f};
Vector3 Vector3_Right = {1.0f, 0.0f, 0.0f};

Vector3 Vector3_Multiply(Vector3 &vec, float scalar) {
    Vector3 result;
    result.x = vec.x * scalar;
    result.y = vec.y * scalar;
    result.z = vec.z * scalar;
    return result;
}

float Vector3_Distance(Vector3 &a, Vector3 &b) {
    float dx = a.x - b.x;
    float dy = a.y - b.y;
    float dz = a.z - b.z;
    return sqrt(dx * dx + dy * dy + dz * dz);
}

Vector3 Vector3_Subtract(Vector3 &a, Vector3 &b) {
    Vector3 result;
    result.x = a.x - b.x;
    result.y = a.y - b.y;
    result.z = a.z - b.z;
    return result;
}

Il2CppObject *GetComponent(Il2CppObject *object, Il2CppClass *type) {
    auto sysType = RET_0_UNLESS(il2cpp_utils::GetSystemType(type));
    // Blame Sc2ad if this simplification doesn't work
    return *RET_0_UNLESS(il2cpp_utils::RunMethod(object, "GetComponent", sysType));
}


void TrickManager::Start() {
    OVRInput = CRASH_UNLESS(il2cpp_utils::GetClassFromName("", "OVRInput"));
    ControllerMask = *CRASH_UNLESS(il2cpp_utils::GetFieldValue<Controller>(
        il2cpp_utils::GetClassFromName("", "OVRInput/Controller"), "All"));

    _rigidBody = CRASH_UNLESS(GetComponent(Saber, il2cpp_utils::GetClassFromName("UnityEngine", "Rigidbody")));
    _collider  = CRASH_UNLESS(GetComponent(Saber, il2cpp_utils::GetClassFromName("UnityEngine", "BoxCollider")));
    _vrPlatformHelper = *CRASH_UNLESS(il2cpp_utils::GetFieldValue(VRController, "_vrPlatformHelper"));

    if (_isLeftSaber) {
        _buttonMapping = ButtonMapping::LeftButtons;
    } else {
        _buttonMapping = ButtonMapping::RightButtons;
    }
}

void TrickManager::Update() {
    if (!_vrPlatformHelper) return;
    ValueTuple trackingPos = GetTrackingPos();
    _controllerPosition = trackingPos.item1;
    _controllerRotation = trackingPos.item2;
    _velocity = Vector3_Subtract(_controllerPosition, _prevPos);
    _saberSpeed = Vector3_Distance(_controllerPosition, _prevPos);
    _prevPos = _controllerPosition;
    if (_getBack) {
        float deltaTime = getDeltaTime();
        float num = 8.0f * deltaTime;
        // saberPos = this.Saber.transform.position;
        auto* saberGO = *CRASH_UNLESS(il2cpp_utils::RunMethod(Saber, "get_gameObject"));
        auto* saberTransform = *CRASH_UNLESS(il2cpp_utils::RunMethod(saberGO, "get_transform"));
        Vector3 saberPos = *CRASH_UNLESS(il2cpp_utils::RunMethod<Vector3>(saberTransform, "get_position"));
        saberPos = *CRASH_UNLESS(il2cpp_utils::RunMethod<Vector3>(
            "UnityEngine", "Vector3", "Lerp", saberPos, _controllerPosition, num));
        CRASH_UNLESS(il2cpp_utils::RunMethod(saberTransform, "set_position", saberPos));
        float num2 = Vector3_Distance(_controllerPosition, saberPos);
        if (num2 < 0.3f) {
            ThrowEnd();
        } else {
            CRASH_UNLESS(il2cpp_utils::RunMethod(saberTransform, "Rotate", Vector3_Right, _saberRotSpeed, RotateSpace));
        }
    }
    CheckButtons();
}

ValueTuple TrickManager::GetTrackingPos() {
    XRNode controllerNode = *CRASH_UNLESS(il2cpp_utils::RunMethod<XRNode>(VRController, "get_node"));
    int controllerNodeIdx = *CRASH_UNLESS(il2cpp_utils::RunMethod<int>(VRController, "get_nodeIdx"));

    ValueTuple result;
    bool nodePose = *CRASH_UNLESS(il2cpp_utils::RunMethod<bool>(_vrPlatformHelper, "GetNodePose", controllerNode,
        controllerNodeIdx, result.item1, result.item2));
    if (!nodePose) {
        result.item1 = {-0.2f, 0.05f, 0.0f};
        result.item2 = {0.0f, 0.0f, 0.0f, 1.0f};
    }
    return result;
}

void TrickManager::CheckButtons() {
    if (!OVRInput) {
        OVRInput = il2cpp_utils::GetClassFromName("", "OVRInput");
    }

    if (!_getBack &&
            *CRASH_UNLESS(il2cpp_utils::RunMethod<bool>(OVRInput, "Get", _buttonMapping.ThrowButton, ControllerMask))) {
        ThrowStart();
    } else if (!_getBack &&
            *CRASH_UNLESS(il2cpp_utils::RunMethod<bool>(OVRInput, "GetUp", _buttonMapping.ThrowButton, ControllerMask))) {
        ThrowReturn();
    } else if (!_isThrowing && !_getBack &&
            *CRASH_UNLESS(il2cpp_utils::RunMethod<bool>(OVRInput, "Get", _buttonMapping.RotateButton, ControllerMask))) {
        InPlaceRotation();
    } else if (_isRotatingInPlace &&
            *CRASH_UNLESS(il2cpp_utils::RunMethod<bool>(OVRInput, "GetUp", _buttonMapping.RotateButton, ControllerMask))) {
        InPlaceRotationEnd();
    }
}


void TrickManager::ThrowStart() {
    log(DEBUG, "%s throw start!", _isLeftSaber ? "Left" : "Right");
    if (!_isThrowing) {
        CRASH_UNLESS(il2cpp_utils::RunMethod(VRController, "set_enabled", false));
        CRASH_UNLESS(il2cpp_utils::RunMethod(_rigidBody, "set_isKinematic", false));
        Vector3 velo = Vector3_Multiply(_velocity, 220.0F);
        CRASH_UNLESS(il2cpp_utils::RunMethod(_rigidBody, "set_velocity", velo));
        CRASH_UNLESS(il2cpp_utils::RunMethod(_collider, "set_enabled", false));
        _saberRotSpeed = _saberSpeed * 400.0f;
        _isThrowing = true;
    }

    ThrowUpdate();
}

void TrickManager::ThrowUpdate() {
    // saberPos = this.Saber.transform.position;
    auto* saberGO = *CRASH_UNLESS(il2cpp_utils::RunMethod(Saber, "get_gameObject"));
    auto* saberTransform = *CRASH_UNLESS(il2cpp_utils::RunMethod(saberGO, "get_transform"));
    CRASH_UNLESS(il2cpp_utils::RunMethod(saberTransform, "Rotate", Vector3_Right, _saberRotSpeed, RotateSpace));
}

void TrickManager::ThrowReturn() {
    if (_isThrowing) {
        CRASH_UNLESS(il2cpp_utils::RunMethod(_rigidBody, "set_isKinematic", true));
        CRASH_UNLESS(il2cpp_utils::RunMethod(_rigidBody, "set_velocity", Vector3_Zero));
        _getBack = true;
        _isThrowing = false;
    }
}

void TrickManager::ThrowEnd() {
    _getBack = false;
    CRASH_UNLESS(il2cpp_utils::RunMethod(_collider, "set_enabled", true));
    CRASH_UNLESS(il2cpp_utils::RunMethod(VRController, "set_enabled", true));
}


void TrickManager::InPlaceRotationStart() {
    log(DEBUG, "%s rotate start!", _isLeftSaber ? "Left" : "Right");
    _currentRotation = 0.0f;
    CRASH_UNLESS(il2cpp_utils::RunMethod(VRController, "set_enabled", false));
    _isRotatingInPlace = true;
}

void TrickManager::InPlaceRotationEnd() {
    _isRotatingInPlace = false;
    CRASH_UNLESS(il2cpp_utils::RunMethod(VRController, "set_enabled", true));
}

void TrickManager::InPlaceRotation() {
    if (!_isRotatingInPlace) {
        InPlaceRotationStart();
    }

    auto* saberGO = *CRASH_UNLESS(il2cpp_utils::RunMethod(Saber, "get_gameObject"));
    auto* saberTransform = *CRASH_UNLESS(il2cpp_utils::RunMethod(saberGO, "get_transform"));
    CRASH_UNLESS(il2cpp_utils::RunMethod(saberTransform, "set_rotation", _controllerRotation));
    CRASH_UNLESS(il2cpp_utils::RunMethod(saberTransform, "set_position", _controllerPosition));
    _currentRotation -= 18.0F;
    CRASH_UNLESS(il2cpp_utils::RunMethod(saberTransform, "Rotate", Vector3_Right, _currentRotation, RotateSpace));
}
