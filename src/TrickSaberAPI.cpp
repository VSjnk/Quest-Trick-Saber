#include "TrickManager.hpp"
#include "TrickSaberAPI.hpp"




TrickManager& getTrickManager(GlobalNamespace::SaberType saberType) {
    if (saberType == GlobalNamespace::SaberType::SaberA) {
        return leftSaber;
    } else
        return rightSaber;
}

extern "C" TrickSaber::TrickState getThrowState(GlobalNamespace::SaberType saberType) {
    TrickManager& trickManager = getTrickManager(saberType);

    return trickManager.getThrowState();
}

extern "C" TrickSaber::TrickState getSpinState(GlobalNamespace::SaberType saberType) {
    TrickManager& trickManager = getTrickManager(saberType);

    return trickManager.getSpinState();
}

extern "C" UnityEngine::GameObject* getActiveSaber(GlobalNamespace::SaberType saberType) {
    TrickManager& trickManager = getTrickManager(saberType);

    return trickManager.getActiveSaber();
}

extern "C" UnityEngine::GameObject* getTrickSaber(GlobalNamespace::SaberType saberType) {
    TrickManager& trickManager = getTrickManager(saberType);

    return trickManager.getTrickSaber();
}

extern "C" UnityEngine::GameObject* getNormalSaber(GlobalNamespace::SaberType saberType) {
    TrickManager& trickManager = getTrickManager(saberType);

    return trickManager.getNormalSaber();
}
