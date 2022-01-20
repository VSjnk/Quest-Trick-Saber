#pragma once

// Ignore can't be found
#include <dlfcn.h>
#include "GlobalNamespace/AudioTimeSyncController.hpp"
#include "beatsaber-hook/shared/utils/logging.hpp"
#include "beatsaber-hook/shared/utils/hooking.hpp"
#include "GlobalNamespace/SaberClashChecker.hpp"
#include "GlobalNamespace/SaberManager.hpp"
#include "GlobalNamespace/Saber.hpp"
#include "UnityEngine/Transform.hpp"
#include "UnityEngine/GameObject.hpp"

inline std::vector<int64_t> objectDestroyTimes;
inline int objectCount = 0;

#include "questui/shared/QuestUI.hpp"
#include "questui/shared/BeatSaberUI.hpp"

static ModInfo modInfo;
const Logger& logger();
static GlobalNamespace::AudioTimeSyncController *audioTimeSyncController = nullptr;
static GlobalNamespace::SaberManager *saberManager = nullptr;

void DisableBurnMarks(int saberType);
void EnableBurnMarks(int saberType, bool force = false);

void SaberManualUpdate(GlobalNamespace::Saber* saber);

static int64_t getTimeMillis() {
    auto time = std::chrono::high_resolution_clock::now();

    return std::chrono::duration_cast<std::chrono::milliseconds>(time.time_since_epoch()).count();
}

// Include the modloader header, which allows us to tell the modloader which mod this is, and the version etc.
#include "modloader/shared/modloader.hpp"

// beatsaber-hook is a modding framework that lets us call functions and fetch field values from in the game
// It also allows creating objects, configuration, and importantly, hooking methods to modify their values
#include "beatsaber-hook/shared/utils/logging.hpp"
#include "beatsaber-hook/shared/config/config-utils.hpp"
#include "beatsaber-hook/shared/utils/il2cpp-functions.hpp"

// Define these functions here so that we can easily read configuration and log information from other files
Configuration &getConfig();
Logger &getLogger();