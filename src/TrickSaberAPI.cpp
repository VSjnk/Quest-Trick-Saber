#include "TrickManager.hpp"
#include "TrickSaberAPI.hpp"




TrickManager& getTrickManager(GlobalNamespace::SaberType saberType) {
    if (saberType == GlobalNamespace::SaberType::SaberA) {
        return leftSaber;
    } else
        return rightSaber;
}

EXPOSE_API(getThrowState, TrickSaber::TrickState, GlobalNamespace::SaberType saberType) {
    TrickManager& trickManager = getTrickManager(saberType);

    return trickManager.getThrowState();
}

EXPOSE_API(getSpinState, TrickSaber::TrickState, GlobalNamespace::SaberType saberType) {
    TrickManager& trickManager = getTrickManager(saberType);

    return trickManager.getSpinState();
}

EXPOSE_API(getActiveSaber, UnityEngine::GameObject*, GlobalNamespace::SaberType saberType) {
    TrickManager& trickManager = getTrickManager(saberType);

    return trickManager.getActiveSaber();
}

EXPOSE_API(getTrickSaber, UnityEngine::GameObject*, GlobalNamespace::SaberType saberType) {
    TrickManager& trickManager = getTrickManager(saberType);

    return trickManager.getTrickSaber();
}

EXPOSE_API(getNormalSaber, UnityEngine::GameObject*, GlobalNamespace::SaberType saberType) {
    TrickManager& trickManager = getTrickManager(saberType);

    return trickManager.getNormalSaber();
}
