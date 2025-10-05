![Discord Shield](https://discord.com/api/guilds/1041603212798599168/widget.png?style=shield) [Chat with us on Discord!](https://discord.gg/rp5cBDtGuM)

[![Twitter](https://upload.wikimedia.org/wikipedia/commons/thumb/6/6f/Logo_of_Twitter.svg/20px-Logo_of_Twitter.svg.png)](https://twitter.com/BeatDropVis) [@BeatDropVis](https://twitter.com/BeatDropVis)

![BeatDrop Logo](https://github.com/OfficialIncubo/The-Improved-and-Modified-Version-of-BeatDrop-Music-Visualizer/raw/master/Logos/BeatDrop%20New%20Logo%20GitGub%20ver.png) (logo design by Incubo_)

# BeatDrop Music Visualizer

BeatDrop Music Visualizer is an improved standalone version of the [original inactive repository fork](https://github.com/mvsoft74/BeatDrop) based of the [Original MilkDrop2 Plug-in](https://www.geisswerks.com/milkdrop/) for [Winamp](https://winamp.com) that aims to add better features and bug fixes/optimizations for versatility, usability and amazing. This is also considered as MilkDrop improvement.

Highlights:
- Based of the [Original MilkDrop Plug-in](https://www.geisswerks.com/milkdrop/).
- Perfectly adjusted beat detection for better audio reaction (also configurable)
- New waveforms and transitions
- 16 custom shapes and waves limit
- Shader Precaching or Caching for Instant Preset Loading
- [Spout](https://spout.zeal.co) integration
- [projectM-eval](https://github.com/projectM-visualizer/projectm-eval) library integration
- Hard Cut Modes, Transparency Mode, Playback Controls, Startup Preset etc.
- And so much more!

# FOR MACOS USERS

Sorry, I have not tested running BeatDrop on a MacOS, but you can try running it with [Parallels Desktop](https://www.parallels.com) or [similar virtualization tools](https://www.youtube.com/watch?v=dHnw-K1-Ui8), such as [UTM (known as Universal Turing Machine)](https://mac.getutm.app)

# FEATURES

## +9 ADDITIONAL SIMPLE WAVEFORMS!
Enjoy these new simple waveforms with most crazy and fascinating patterns! Made by community, made for you.

![BeatDropNewWaveformsDemo](https://github.com/user-attachments/assets/ad90e429-f556-4d27-8dc7-fb26f3dfc02e)

---------------------------------------------------------------------------------------------------------------------------------------------
## UP TO 16 CUSTOM WAVES AND SHAPES!
Now you have more possibilities to create/edit some custom waves or shapes with up to 16 slots! This will get much chaotic than before.

![BeatDrop16ShapesMilkDrop2077Demo](https://github.com/user-attachments/assets/81ddcd0e-754a-466a-9749-120ac0aaf8b7)

---------------------------------------------------------------------------------------------------------------------------------------------
## INTEGRATED WITH [SPOUT](https://spout.zeal.co/) FOR SHARING VISUALS EVERYWHERE!
Now you can send the BeatDrop's Visual Renderer via Spout to [OBS](https://obsproject.com) ([plug-in](https://obsproject.com/forum/resources/obs-spout-bi-directional-video-sharing-with-obs-studio-from-any-spout-program.1384) needed), [Resolume](https://www.resolume.com/software), [NestDrop](https://nestimmersion.ca/nestdrop.php), [TouchDesigner](https://derivative.ca), [MadMapper](https://madmapper.com), [Unity](https://unity.com) ([KlakSpout](https://github.com/keijiro/KlakSpout) needed), [HeavyM](https://www.heavym.net) and so much more that support it. Simply press CTRL + Z and you're good to go!

List of available apps here: https://spout.zeal.co

![BeatDropSpoutDemo](https://github.com/user-attachments/assets/5abf19b8-d064-4a4f-b756-d1936fdc88dc) ![BeatDropSpoutResolumeArenaDemo](https://github.com/user-attachments/assets/63ef3d28-4244-40d7-88a4-848cd3751cf3)

---------------------------------------------------------------------------------------------------------------------------------------------
## SCREEN-DEPENDENT RENDER MODE & PRESET PATCH FIXES
Bring back the nostalgia from 2000s by fixing some old MilkDrop 1 presets behavior with Screen-Dependent Render Mode, making the visual way better. You are free to turn it off if you don't want it.

I've also manually patch fixed all the bugged presets to look very/almost identical to MilkDrop 1/2.0.

![BeatDropScreenDependentRenderModeDemo](https://github.com/user-attachments/assets/85b93017-178b-4b13-8be4-1998872784fd)

---------------------------------------------------------------------------------------------------------------------------------------------
## SHADER PRECACHING/CACHING
Getting in stress if the visual is freezing while the shaders are compiling? Don't worry. I've got you! BeatDrop now supports shader precaching/caching for instant loading presets. Now you can experience 0 seconds compile time when you loaded the same preset that contains shaders! ;)

Here is the comparison below!

![BeatDropShaderCacheComparison](https://github.com/user-attachments/assets/16bcc5a4-e9b2-4bb1-a511-c85f63648465)

---------------------------------------------------------------------------------------------------------------------------------------------
## BORDERLESS MODE
Need any media players that needs a music visualizer on top? Of course! Just run BeatDrop, press F2, adjust or fit to any part of the window in your needs and you're good to relax! You can also start BeatDrop with this feature on by modifying the .ini setting file (beatdrop.ini).

<img width="1493" height="751" alt="BeatDropBorderlessModeDemo" src="https://github.com/user-attachments/assets/48e4d8c0-45a9-4589-8a20-2f3aa7866e3d" /> <img width="1376" height="889" alt="BeatDropBorderlessModeOnSpotifyDemo" src="https://github.com/user-attachments/assets/4ae6cd0c-646c-487f-bbac-19ebce42d7d5" />

---------------------------------------------------------------------------------------------------------------------------------------------
## PIXEL SHADER VERSION 4 PRESETS SUPPORT FOR [AMD/ATI]((https://www.amd.com/en.html)) PROCESSOR/GRAPHIC CARD
After 20 years of the bug when using any Pixel Shader Version 4 presets that cause stutters/freezes on one of the shaders, We have researched to fix this bug & making it support for AMD/ATI cards. Now you don't worry about the shader bottleneck.

<img width="1920" height="1080" alt="BeatDropAMDATISupportV2" src="https://github.com/user-attachments/assets/c29cc402-0854-495d-a143-df385f6c9840" />

This is a BETA feature. Only a few presets can get errors about complicated shader code, e.g. martin - Gin Tonic on Ice, martin - enlighten me etc. If anyone wants to get it fully work, please open a PR and check the issue [#38](https://github.com/OfficialIncubo/BeatDrop-Music-Visualizer/issues/38).

(not affiliated with Advanced Micro Devices, Inc. or ATI Technologies Inc.)

---------------------------------------------------------------------------------------------------------------------------------------------
## HI-RES AUDIO SUPPORT (96kHz, 192kHz and beyond)
7 years ago, BeatDrop had it's issue that it resulted a loss of frequency response or a complete non-reaction when you set the sample rate above 44100Hz or 48000Hz. So, I've made support for higher sample rates (from 96000Hz to beyond) by downsampling it to the visual-preferred one, because MilkDrop is designed for 44100Hz sample rate.

![hi-res-audio-sound-waves](https://github.com/user-attachments/assets/7b80210c-a7c4-4c84-9cb4-8bfbc3eca8ea)

---------------------------------------------------------------------------------------------------------------------------------------------
## CONTROL PRESETS BY USING A MOUSE
To make BeatDrop more and more interactive, Me & [MilkDrop2077/Serge Blanc](https://x.com/MilkDrop2077) have made support for controlling presets by just using a mouse! Preset authors can now use the mouse variables in a simple way. Just pass them to init, per-frame, per-vertex or shaders, use a mouse and look how it goes! For more information about this, you can read it from docs folder.

![BeatDropMouseSupportDemo](https://github.com/user-attachments/assets/5184e575-26cf-4dc9-a2cb-22fa59656c8e)

---------------------------------------------------------------------------------------------------------------------------------------------
## REAL-TIME SONG INFORMATION
Now it can get the song information from any media players using SMTC, sending it to visualizer. You can also show/hide this on the bottom-left corner by pressing F8.

![BeatDropRealTimeSongInfoDemo](https://github.com/user-attachments/assets/6cc5cb3d-f82e-4526-b686-94670e3483b8)

Note that it only works for Windows 10 or Windows 11. For Windows Vista, 7, 8 and 8.1 users, please run BeatDrop_OldOS.exe, which it doesn't use this feature and makes it better support for old Windows OS.

---------------------------------------------------------------------------------------------------------------------------------------------
## INTEGRATED WITH [PROJECTM-EVAL](https://github.com/projectM-visualizer/projectm-eval) FOR OPTIMIZATION
BeatDrop now features [projectM Expression Evaluation Library](https://github.com/projectM-visualizer/projectm-eval), which replaces [Nullsoft Expression Evaluation Library](https://github.com/projectM-visualizer/milkdrop2/tree/master/src/ns-eel2) for portability, compile time + speed optimization and assembly-free.

---------------------------------------------------------------------------------------------------------------------------------------------
## NO AUDIO CUTOFF WHILE AUDIO OUTPUT DEVICE CHANGE, SAMPLE RATE CHANGE OR DISCONNECTION
To not break the listening sessions or DJ sessions, BeatDrop will no longer interrupts the audio reaction while the audio device is changed, sample rate is changed or the device disconnection happens. Also no need to restart. ;)

# BEFORE COMPILING THE CODE

* First of all, [Visual Studio 2022](https://visualstudio.microsoft.com/vs) is now required.
* [DirectX SDK](https://www.microsoft.com/en-us/download/details.aspx?id=6812) is even required.
* Workloads required before compiling:
![Workloads required to use with Visual Studio 2022](https://github.com/user-attachments/assets/7bc967d5-3683-4180-a1ad-8a99a855d5fc)
* CMake is now obligatory to install! Look at the name from Desktop development with C++ category below and check this box. [projectM-eval](https://github.com/projectM-visualizer/projectm-eval) library is now integrated in.

![CMake optional library checked](https://github.com/user-attachments/assets/219d03a0-3c16-42ef-a05d-1374ae7ee3a8)

* After that, please change from debug to release and build it.

# HOW TO COMPILE THE CODE AND UPDATE/INTEGRATE PROJECTM-EVAL LIBRARY CORRECTLY

BeatDrop now uses [projectM-eval](https://github.com/projectM-visualizer/projectm-eval) library, a drop-in replacement of Nullsoft Expression Evaluation Library, which it's assembly-free and it uses much faster instructions than i386 instructions that achieves preset compilation performance optimization. [projectM-eval](https://github.com/projectM-visualizer/projectm-eval) also performs a few compile-time optimizations like replacing larger constant expressions with a simple value.

Steps on how to compile BeatDrop with [projectM-eval](https://github.com/projectM-visualizer/projectm-eval) library:

## First method: Using [vcpkg](https://vcpkg.io)

1. Vcpkg options from the project configuration are enabled by default. If you want to disable them for building as local files, look at the second method.
2. First update your baseline by clicking View -> Terminal, then type vcpkg x-update-baseline.
3. Compile your code. Vcpkg automatically generates the projectM-eval library using CMake.
4. It will receive an error about one of the libraries: `Invalid or corrupt file: cannot read at 0x536E` (projectM_ns-eel2.lib).
5. The generated files are in vcpkg_installed folder. Copy both libraries from vcpkg_installed\x86-windows-static\x86-windows-static\lib, then paste them to lib folder from the main source code folder.
6. (Optional) Copy the ns-eel header from the same step from 5., but in include\projectm-eval\ns-eel2, then paste it to ns-eel-shim folder, still from the main source code folder.
7. Now build/compile the code and you are ready to go. If you see that your output gives "1 up to date", just delete BeatDrop.exe from vis_milk2 -> Release folder. Compile it again.
8. Check your generated .exe file in vis_milk2/Release/BeatDrop.exe, then copy it in your BeatDrop folder.

## Second method: Using [CMake](https://cmake.org) (for latest version build)

Before this, [Git](https://git-scm.com) is required for cloning repositories.

1. Make sure you have disabled the vcpkg feature (if you want). First go to Project from toolbar, click properties, then go to Configuration Properties -> vcpkg, then turn the first two options off. It should look like this:
<img width="786" height="544" alt="image" src="https://github.com/user-attachments/assets/f1351a8b-e03c-4379-a13b-ae3804fd9c3b" />

If you want to turn back on, do the same step, then change them to "Yes".

2. Use [VS2022](https://visualstudio.microsoft.com/vs)'s terminal (View from toolbar -> Terminal) or PowerShell. Clone the repository as a local package.
```
git clone https://github.com/projectM-visualizer/projectm-eval.git
cd projectm-eval
git fetch --all (if needed)
```
3. Go to your main code folder -> projectm-eval -> CMakeLists.txt, look at BUILD_NS_EEL_SHIM option, then turn it on.

For example: `option(BUILD_NS_EEL_SHIM "Build and install the ns-eel2 compatibility API shim." ON)`

4. Build and install the projectM-eval library using CMake.
```
mkdir your_build_folder
cd your_build_folder
cmake -A Win32 ..
cmake --build . --config "Release" --parallel
cmake --install . --prefix your-installation-folder-name-here.
```
5. Now do the same steps as step 4 and 5, but look at your main code folder -> projectm-eval -> your build folder -> your CMake installation folder you provided, then there are include headers and library files installed in. If you want to copy only header or library files, then look at them inside.
6. Now build/compile and it should be working. Again, look at the step 8 from the first method after you built/compiled.

If you have problems, ask me on Discord, Twitter or Instagram.

# BEFORE YOU RUN BEATDROP

Please download and install [DirectX End-User Runtime Web](https://www.microsoft.com/en-us/download/details.aspx?id=35) first.

Microsoft Visual C++ 2015-2022 Redistributable is also required to run. You can get it from [system requirements](https://github.com/OfficialIncubo/BeatDrop-Music-Visualizer#system-requirements) category.

BeatDrop now has a Spout integration! It's now supported to route the visual to any apps, such as [NestDrop](https://nestimmersion.ca/nestdrop.php), [Resolume Arena](https://www.resolume.com/software/avenue-arena), [OBS](https://obsproject.com) ([as plug-in](https://github.com/Off-World-Live/obs-spout2-plugin)) and so much more.

Download Spout: https://spout.zeal.co

# HOW TO USE BEATDROP

1. Easily run BeatDrop!
2. Open [YouTube](https://www.youtube.com), [Spotify](https://open.spotify.com), [AIMP](https://www.aimp.ru) or any media players/audio sources and let them play! (Yes! It reacts to all the audio sources)
3. Watch them react and enjoy! 👌🪩

Get started by pressing F1 to get a list of hotkeys and functionalities.

# USAGE

You can use BeatDrop for parties, karaoke, music videos, DJ mixes and so on, but you must credit this repo if you used this! I am not even angry anymore if you didn't credit this, but use #beatdrop, #beatdropmusicvisualizer or BeatDrop as tags, then I'll look at them.

# MILKDROP MEGAPACK - GET ALL THE MILKDROP PRESETS

If you want to have all the MilkDrop presets in your own, just download [this MegaPack](https://drive.google.com/file/d/1DlszoqMG-pc5v1Bo9x4NhemGPiwT-0pv/view), then extract them to your BeatDrop folder (ex: to BeatDrop Resources folder).

# FEATURED WEBSITES

[![Softpedia](https://upload.wikimedia.org/wikipedia/commons/f/f4/Softpedia_logo.svg)](https://www.softpedia.com/get/Multimedia/Audio/Other-AUDIO-Tools/BeatDrop-Music-Visualizer.shtml)

[![AlternativeTo](https://alternativeto.net/static/icons/a2/org-icon.png)](https://alternativeto.net/software/beatdrop/about/)

[<img src="https://vjun.io/uploads/organization/profile_image/1/aa1adb2e-fff9-4492-9544-7f15dd17152e.png" alt="VJ Union" style="width:20%; height:auto;">](https://vjun.io/vdmo/beatdrop-music-visualiser-with-spout-1hof)

[<img src="https://www.topdownload.club/img/logo.png" alt="TopDownload.Club" style="width:30%; height:auto;">](https://win.topdownload.club/beatdrop-music-visualizer-free-download-download.html)

[The Audio File](https://audio-file.org/2023/10/13/beatdrop-music-visualizer/)

[Spout Open-Source Projects](https://leadedge.github.io/spout-projects.html#OfficialIncubo)

[Opensource-DVD](https://www.opensource-dvd.de/programme/beatdrop.htm)

[Never Complete Only Abandoned](https://publish.obsidian.md/xybre/permalink/73b8a0f5-2464-4404-929b-42a5d3c451fd)

# FOLLOW ME ON ANY SOCIAL NETWORKS

[<img src="https://static.vecteezy.com/system/resources/previews/042/148/611/non_2x/new-twitter-x-logo-twitter-icon-x-social-media-icon-free-png.png" alt="X" style="width:10%; height:75%;">](https://x.com/BeatDropVis)
[<img src="https://static.vecteezy.com/system/resources/previews/018/930/718/non_2x/discord-logo-discord-icon-transparent-free-png.png" alt="Discord (Join server)" style="width:10%; height:75%;">](https://discord.gg/rp5cBDtGuM)

---------------------------------------------------------------------------------------------------------------------------------------------

# ORIGINAL REPOSITORY DESCRIPTION

BeatDrop is a stand-alone implementation of the amazing Milkdrop2 Winamp plug-in.
It lets you experience the stunning visual 2D effects with your music player of choice. No additional configuration steps needed! Just start BeatDrop and play your music.
Use BeatDrop with your favourite:

* Music player:
  [foobar2000](https://www.foobar2000.org/),
  [VLC media player](https://www.videolan.org/vlc/index.html),
  [Clementine](https://www.clementine-player.org/),
  [AIMP](https://www.aimp.ru/),
  ...
* Web-based player:
  [SoundCloud](https://soundcloud.com/),
  [YouTube](https://www.youtube.com/),
  [Vimeo](https://vimeo.com/),
  ...
* Internet Radio station:
  [SomaFM](https://somafm.com/),
  [DI.FM](https://www.di.fm/),
  [RauteMusik.FM](https://www.rm.fm/),
  ...

# System Requirements
* For building without SMTC (old OS build): Windows Vista, Windows 7, Windows 8, Windows 8.1, Windows 10, Windows 11
* For building with SMTC (normal build): Windows 10, Windows 11
* Minimum 2GB of RAM required
* WASAPI - compatible sound card
* DirectX 9 or higher - compatible GPU
* DirectX End-User [Runtimes](https://www.microsoft.com/en-us/download/details.aspx?id=8109) (also included in the installer) contains the required 32-bit helper libraries d3dx9_43.dll and d3dx9_31.dll
* [Microsoft Visual C++ 2015-2022 Redistributable](https://www.microsoft.com/en-us/download/details.aspx?id=52685) or (optional) [Microsoft Visual C++ Redistributable All-in-One](https://www-techpowerup-com.cdn.ampproject.org/v/s/www.techpowerup.com/download/visual-c-redistributable-runtime-package-all-in-one/?amp=&amp_gsa=1&amp_js_v=a9&usqp=mq331AQIUAKwASCAAgM%3D#amp_tf=De%20la%20%251%24s&aoh=17542342520002&referrer=https%3A%2F%2Fwww.google.com&ampshare=https%3A%2F%2Fwww.techpowerup.com%2Fdownload%2Fvisual-c-redistributable-runtime-package-all-in-one%2F)

# Acknowledgements
Special thanks to:

* Ryan Geiss & Rovastar (John Baker) - [Official Milkdrop2 Source Code](https://sourceforge.net/projects/milkdrop2/)
* [oO-MrC-Oo](https://github.com/oO-MrC-Oo) - [XBMC Plug-in](https://github.com/oO-MrC-Oo/Milkdrop2-XBMC)
* Casey Langen ([@clangen](https://github.com/clangen)) - [milkdrop2-musikcube](https://github.com/clangen/milkdrop2-musikcube)
* Matthew van Eerde ([@mvaneerde](https://github.com/mvaneerde)) - [loopback-capture](https://github.com/mvaneerde/blog)
* projectM Team ([Kai Blaschke](https://github.com/kblaschke)) - [projectM-eval](https://github.com/projectM-visualizer/projectm-eval), contributor
* Lynn Jarvis ([@leadedge](https://github.com/leadedge)) - [BeatDrop for Spout](https://github.com/leadedge/BeatDrop), [Spout](https://spout.zeal.co), contributor
* Patrick Pomerleau @ Nest Immersion ([@nestdome](https://www.instagram.com/nestdome)) - help, contributor, feature integrations to NestDrop
* [IkeC](https://github.com/IkeC) - contributor, [Milkwave](https://github.com/IkeC/Milkwave) Developer
* [MilkDrop2077](https://github.com/milkdrop2077) - contributor
* Me, [u/Decent-Tangerine4998](https://www.reddit.com/user/Decent-Tangerine4998) - Tester
* ...and all the preset authors and contributors!

# License

[license]: #license

This repository is licensed under the 3-Clause BSD License ([LICENSE](LICENSE) or [https://opensource.org/licenses/BSD-3-Clause](https://opensource.org/licenses/BSD-3-Clause)) with the exception of where otherwise noted.

Although the original Matthew van Eerde's [loopback-capture](https://github.com/mvaneerde/blog) project didn't explicitly state the license, the author has been kind enough to provide the [license clarification](
https://blogs.msdn.microsoft.com/matthew_van_eerde/2014/11/05/draining-the-wasapi-capture-buffer-fully/)

> ## Sunny March 29, 2015 at 11:06 pm
> Hi. Just wondering is this open source? I'm looking for something like this for my school project.
>
> ## Maurits [MSFT] March 30, 2015 at 8:35 am
> @Sunny do with the source as you like.

All changes in this repository to the original Matthew's code are published either under the terms of BSD license or the license provided by original author.

# Contributions

Unless you explicitly state otherwise, any contribution intentionally submitted for inclusion in the work by you, shall be licensed as above.
