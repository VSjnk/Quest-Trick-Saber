#pragma once

#include <dlfcn.h>
#include <stdlib.h>
#include <unordered_map>
#include <unordered_set>
#include <UnityEngine/BoxCollider.hpp>

#include "beatsaber-hook/shared/utils/typedefs.h"
#include "AllEnums.hpp"
#include "InputHandler.hpp"
#include "SaberTrickModel.hpp"
#include "GlobalNamespace/VRController.hpp"
#include "GlobalNamespace/IVRPlatformHelper.hpp"
#include "GlobalNamespace/HapticFeedbackController.hpp"
#include "GlobalNamespace/HapticFeedbackController_RumbleData.hpp"
#include "GlobalNamespace/Saber.hpp"
#include "UnityEngine/Transform.hpp"

#include <experimental/coroutine>
#include "custom-types/shared/coroutine.hpp"
#include "sombrero/shared/QuaternionUtils.hpp"

// Conventions:
// tX means the type (usually System.Type i.e. Il2CppReflectionType) of X
// cX means the Il2CppClass of X
// xT means the .transform of unity object X


const Sombrero::FastVector3 Vector3_Zero = Sombrero::FastVector3(0.0f, 0.0f, 0.0f);
const Sombrero::FastVector3 Vector3_Right = Sombrero::FastVector3(1.0f, 0.0f, 0.0f);
const Sombrero::FastQuaternion Quaternion_Identity = Sombrero::FastQuaternion(0.0f, 0.0f, 0.0f, 1.0f);


struct ValueTuple {
    Sombrero::FastVector3 item1;
    Sombrero::FastQuaternion item2;
};


struct ButtonMapping {
    public:
		bool left;
		// static ButtonMapping LeftButtons;
		// static ButtonMapping RightButtons;
		std::unordered_map<int, std::unordered_set<std::unique_ptr<InputHandler>>> actionHandlers;

		void Update();

        ButtonMapping() {};
        ButtonMapping(bool isLeft) {
			left = isLeft;
			Update();
        };
};

static GlobalNamespace::HapticFeedbackController* _hapticFeedbackController;  	// ::HapticFeedbackController

class TrickManager {
    public:
        void LogEverything();
        bool _isLeftSaber = false;
        GlobalNamespace::Saber* Saber;         // ::Saber
        GlobalNamespace::VRController* VRController = nullptr;  // ::VRController
		TrickManager* other = nullptr;
		static void StaticClear();
		void Clear();
		void Start();
		void EndTricks();
		static void StaticPause();
		void PauseTricks();
		static void StaticResume();
    	void ResumeTricks();
		static void StaticFixedUpdate();
		void FixedUpdate();
        void Update();
        [[nodiscard]] bool isDoingTricks() const;

        [[nodiscard]] TrickSaber::TrickState getThrowState() const;
        [[nodiscard]] TrickSaber::TrickState getSpinState() const;
        [[nodiscard]] UnityEngine::GameObject* getActiveSaber() const;
        [[nodiscard]] UnityEngine::GameObject* getTrickSaber() const;
        [[nodiscard]] UnityEngine::GameObject* getNormalSaber() const;

        [[nodiscard]] SaberTrickModel* getTrickModel() const;

	protected:
        TrickSaber::TrickState _throwState;  // initialized in Start
        TrickSaber::TrickState _spinState;

    private:
        void setSpinState(TrickSaber::TrickState state);
        void setThrowState(TrickSaber::TrickState state);
		void Start2();
		UnityEngine::Transform * FindBasicSaberTransform();
        ValueTuple GetTrackingPos();
        void CheckButtons();
        void ThrowStart();
        void ThrowUpdate();
        void ThrowReturn();
        void ThrowEnd();
        void InPlaceRotationStart();
		void InPlaceRotation(float power);
		void _InPlaceRotate(float amount);
		custom_types::Helpers::Coroutine LerpToOriginalRotation();
        custom_types::Helpers::Coroutine CompleteRotation();
		void InPlaceRotationReturn();
        void InPlaceRotationEnd();
		void TrickStart() const;
		void TrickEnd() const;
		void AddProbe(const Sombrero::FastVector3& vel, const Sombrero::FastVector3& ang);
        Sombrero::FastVector3 GetAverageVelocity();
        Sombrero::FastVector3 GetAverageAngularVelocity();
        GlobalNamespace::IVRPlatformHelper* _vrPlatformHelper;  			    // ::VRPlatformHelper
        ButtonMapping _buttonMapping;
        UnityEngine::BoxCollider* _collider = nullptr;    		// BoxCollider
        Sombrero::FastVector3       _controllerPosition = Vector3_Zero;
        Sombrero::FastQuaternion    _controllerRotation = Quaternion_Identity;
        Sombrero::FastVector3       _prevPos            = Vector3_Zero;
        Sombrero::FastVector3       _angularVelocity    = Vector3_Zero;
        Sombrero::FastQuaternion    _prevRot            = Quaternion_Identity;
        float         _currentRotation;
        float         _saberSpeed         = 0.0f;
        float         _saberRotSpeed      = 0.0f;
		size_t _currentProbeIndex;
		std::vector<Sombrero::FastVector3> _velocityBuffer;
		std::vector<Sombrero::FastVector3> _angularVelocityBuffer;
		float _spinSpeed;
		float _finalSpinSpeed;
		SaberTrickModel* _saberTrickModel = nullptr;
		float _timeSinceStart = 0.0f;
		UnityEngine::Transform* _originalSaberModelT = nullptr;
		Il2CppString* _saberName = nullptr;
		Il2CppString* _basicSaberName = nullptr;  // only exists up until Start2
		UnityEngine::Transform* _saberT = nullptr;  // needed for effecient Start2 checking in Update
		// Vector3 _VRController_position_offset = Vector3_Zero;
        Sombrero::FastVector3 _throwReturnDirection = Vector3_Zero;
		// float _prevThrowReturnDistance;
		UnityEngine::Transform* _fakeTransform;  // will "replace" VRController's transform during trickCutting throws
		bool doFrozenThrow = false;
};

inline TrickManager leftSaber;
inline TrickManager rightSaber;