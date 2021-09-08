# Builds a .qmod file
& $PSScriptRoot/build.ps1

if ($?) {
    Compress-Archive -Path "./mod.json", "./libs/arm64-v8a/libTrickSaber.so", "./libs/arm64-v8a/libbs-utils.so", "./libs/arm64-v8a/libbeatsaber-hook_2_3_0.so" -DestinationPath "./TrickSaber_v0.1.0.zip" -Update
    Remove-Item "./TrickSaber_v0.3.8.qmod"
    Rename-Item "./TrickSaber_v0.1.0.zip" "./TrickSaber_v0.3.8.qmod"
}
