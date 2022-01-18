# & $PSScriptRoot/build.ps1
# if ($?) {
#     adb push libs/arm64-v8a/libTrickSaber.so /sdcard/Android/data/com.beatgames.beatsaber/files/mods/libtricksaber.so
#     if ($?) {
#         adb shell am force-stop com.beatgames.beatsaber
#         adb shell am start com.beatgames.beatsaber/com.unity3d.player.UnityPlayerActivity
#         if ($args[0] -eq "--log") {
#             $timestamp = Get-Date -Format "MM-dd HH:mm:ss.fff"
#             adb logcat -T "$timestamp" main-modloader:W QuestHook[tricksaber`|v0.2.2]:* AndroidRuntime:E *:S
#         }
#     }
# }
& $PSScriptRoot/build.ps1
if ($?) {
    adb push build/libTrickSaber.so /sdcard/Android/data/com.beatgames.beatsaber/files/mods/libTrickSaber.so
    if ($?) {
        adb shell am force-stop com.beatgames.beatsaber
        adb shell am start com.beatgames.beatsaber/com.unity3d.player.UnityPlayerActivity
        if ($args[0] -eq "--log") {
            $timestamp = Get-Date -Format "MM-dd HH:mm:ss.fff"
            adb logcat -T "$timestamp" main-modloader:W QuestHook[TrickSaber`|v0.3.1]:* AndroidRuntime:E *:S
        }
    }
}