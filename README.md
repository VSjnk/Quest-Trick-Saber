<h1>Open saber vr + Mods!</h1>
<h2>Progress</h2>
<h3>Chroma</h3>
There is full Chroma support*
The only thing you can't do, is control each light with the "_light_id" as it is used in changing environments and Open saber only has 1, but the notes change colour properly!
<h3>Mapping extensions</h3>
Mapping extensions is currently in the works as the walls are placed in a weird way I can't understand, however wall sizeing is fully working along side everything else*
the only things not implemented is archs and chains, as i've never even seen a mapping extension level in v3 anyways.

<h3>Noodle extensions</h3>
While I have code that can make it work, there is no animation support and in general is extremly buggy so It's unimplimented but I want to give it support someday!

<h3>Vivify</h3>
No.

# Quest port of https://github.com/ToniMacaroni/TrickSaber.
# Updated for Beat Saber 1.13.2

Follows https://tonimacaroni.github.io/TrickSaber-Docs/Configuration (config at /sdcard/Android/data/com.beatgames.beatsaber/files/mod_cfgs/TrickSaber.json) with a few differences:
1. Her "EnableCuttingDuringTrick" is my "EnableTrickCutting"
2. Her "IsSpeedVelocityDependent" is my "IsSpinVelocityDependent"

Added options:
1. "ButtonOneAction": action triggered by A or X.
2. "ButtonTwoAction": action triggered by B or Y.

To change your config, simply drag your edited TrickSaber.json into the BMBF upload box (as you did for the mod zip itself).
Or use the QuestUI interface

~~Also, there is no UI, pending https://github.com/zoller27osu/Quest-BSML.~~ Added QuestUI support. 

Changed drastically from the original Quest port by Rugtveit, so please do not bother him.

# Credits:
Ported to Beat Saber 1.13.2 by Fernthedev. Many thanks to the original authors of the Quest port for doing most of the work, and the Discord community for giving lots of support, no fuss. 

This is my first Beat Saber Mod (and successful C++ project) so many bugs and ugly code may be within the project.

Many thanks to:
- RedBrumbler for helping me out with the trails (and dealing with my annoying questions)
- darknight1050 for QuestUI and config-utils documentation
- Everyone else who tested the mod and helped me out :)

# Support the original author:
Michael Zoller, the original author of TrickSaber during the Beatsaber 1.11 era, has had lots of trouble lately due to the pandemic. Due to this, his works have to be managed and maintained by someone else.
Michael has done the majority of work on this mod, so it's only fair to credit him and help support him financially.

I urge you to [support him for his work](https://www.paypal.com/paypalme/zoller27osu?locale.x=en_US) if you can.