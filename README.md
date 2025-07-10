![Discord Shield](https://discord.com/api/guilds/1041603212798599168/widget.png?style=shield) [Chat with us on Discord!](https://discord.gg/rp5cBDtGuM)

[![Twitter](https://upload.wikimedia.org/wikipedia/commons/thumb/6/6f/Logo_of_Twitter.svg/20px-Logo_of_Twitter.svg.png)](https://twitter.com/BeatDropVis) [@BeatDropVis](https://twitter.com/BeatDropVis)

![BeatDrop Logo](https://github.com/OfficialIncubo/The-Improved-and-Modified-Version-of-BeatDrop-Music-Visualizer/raw/master/Logos/BeatDrop%20New%20Logo%20GitGub%20ver.png) (logo design by Incubo_)

# BeatDrop Music Visualizer

BeatDrop Music Visualizer is an improved standalone version of the [original inactive repository fork](https://github.com/mvsoft74/BeatDrop) based of the [Original MilkDrop2 Plug-in](https://www.geisswerks.com/milkdrop/) for [Winamp](https://winamp.com) that aims to add better features and bug fixes/optimizations for versatility, usability and amazing. This is also considered as MilkDrop improvement.

It features:
- Based of the [Original MilkDrop Plug-in](https://www.geisswerks.com/milkdrop/).
- Perfectly adjusted beat detection for better audio reaction (also configurable)
- New waveforms and transitions
- 16 custom shapes and waves limit
- [Spout](https://spout.zeal.co) integration
- [projectM-eval](https://github.com/projectM-visualizer/projectm-eval) library integration
- Hard Cut Modes, Transparency Mode, Playback Controls, Startup Preset etc.
- And so much more!

Before compiling the code:
* Workloads required before compiling:
![Workloads required to use with Visual Studio 2022](https://github.com/user-attachments/assets/7bc967d5-3683-4180-a1ad-8a99a855d5fc)
* CMake is now obligatory to install! Look at the name from Desktop development with C++ category below and check this box. [projectM-eval](https://github.com/projectM-visualizer/projectm-eval) library is now integrated in.

![CMake optional library checked](https://github.com/user-attachments/assets/219d03a0-3c16-42ef-a05d-1374ae7ee3a8)

* Anyone who uses Visual Studio 2022, please change from debug to release and build it.

# HOW TO COMPILE THE CODE AND UPDATE/INTEGRATE PROJECTM-EVAL LIBRARY CORRECTLY

BeatDrop now uses [projectM-eval](https://github.com/projectM-visualizer/projectm-eval) library, a drop-in replacement of Nullsoft Expression Evaluation Library, which it's assembly-free and it uses much faster instructions than i386 instructions that achieves preset compilation performance optimization. [projectM-eval](https://github.com/projectM-visualizer/projectm-eval) also performs a few compile-time optimizations like replacing larger constant expressions with a simple value.

Steps on how to compile BeatDrop with [projectM-eval](https://github.com/projectM-visualizer/projectm-eval) library:

1. First update your baseline by clicking View -> Terminal, then type vcpkg x-update-baseline.
2. Compile your code. Vcpkg automatically generates the projectM-eval library using CMake.
3. It will receive an error about one of the libraries: `Invalid or corrupt file: cannot read at 0x536E` (projectM_ns-eel2.lib).
4. The generated files are in vcpkg_installed folder. Copy both libraries from vcpkg_installed\x86-windows-static\x86-windows-static\lib, then paste them to lib folder from the main source code folder.
5. (If needed) Copy the ns-eel header from the same step from 3., but in include\projectm-eval\ns-eel2, then paste it to ns-eel-shim folder, still from the main source code folder.
6. Now compile the code and you are ready to go. If you see that your output gives "1 up to date", just delete BeatDrop.exe from vis_milk2 -> Release folder. Compile it again.
7. It will receive `The command "copy /y /v "..\vis_milk2\Release\BeatDrop.exe" "..\BeatDrop\BeatDrop.exe"`, but it built succesfully. It tries to copy the generated .exe file to BeatDrop folder, which it doesn't exist. Ignore it. Please check your generated .exe file in vis_milk2/Release/BeatDrop.exe, then copy it in your BeatDrop folder.
8. If you have problems, ask me on Discord, Twitter or Instagram.

# BEFORE YOU RUN BEATDROP

Please download and install [DirectX End-User Runtime Web](https://www.microsoft.com/en-us/download/details.aspx?id=35) first.

BeatDrop now has a Spout integration! It now only supported to route the visual to any apps, such as [NestDrop](https://nestimmersion.ca/nestdrop.php), [OBS](https://obsproject.com) ([as plug-in](https://github.com/Off-World-Live/obs-spout2-plugin)) etc.

Download Spout: https://spout.zeal.co

# HOW TO USE BEATDROP

1. First, use any audio output if you want before running

![Audio Output Settings](https://github.com/user-attachments/assets/fd611b87-a62a-4769-9c24-72ce333f38f2)

2. Easily run BeatDrop!
3. If you did the first step, change it back to the default audio output, instead remain what it was.
4. Open [YouTube](https://www.youtube.com), [Spotify](https://open.spotify.com), [AIMP](https://www.aimp.ru) or any media players/audio sources and let them play! (Yes! It reacts to all the audio sources)
5. Watch them react and enjoy! 👌🪩

# USAGE

You can use BeatDrop for parties, karaoke, music videos, DJ mixes and so on, but you must credit this repo if you used this! I am not even angry anymore if you didn't credit this, but use #beatdrop, #beatdropmusicvisualizer or BeatDrop as tags, then I'll look at them.

# Featured Websites:

[![Softpedia](https://upload.wikimedia.org/wikipedia/commons/f/f4/Softpedia_logo.svg)](https://www.softpedia.com/get/Multimedia/Audio/Other-AUDIO-Tools/BeatDrop-Music-Visualizer.shtml)

[![AlternativeTo](https://alternativeto.net/static/icons/a2/org-icon.png)](https://alternativeto.net/software/beatdrop/about/)

[<img src="https://vjun.io/uploads/organization/profile_image/1/aa1adb2e-fff9-4492-9544-7f15dd17152e.png" alt="VJ Union" style="width:20%; height:auto;">](https://vjun.io/vdmo/beatdrop-music-visualiser-with-spout-1hof)

[<img src="https://www.topdownload.club/img/logo.png" alt="TopDownload.Club" style="width:30%; height:auto;">](https://win.topdownload.club/beatdrop-music-visualizer-free-download-download.html)

[The Audio File](https://audio-file.org/2023/10/13/beatdrop-music-visualizer/)

[Spout Open-Source Projects](https://leadedge.github.io/spout-projects.html#OfficialIncubo)

[Opensource-DVD](https://www.opensource-dvd.de/programme/beatdrop.htm)

# Follow me on any social networks:

[<img src="https://static.vecteezy.com/system/resources/previews/042/148/611/non_2x/new-twitter-x-logo-twitter-icon-x-social-media-icon-free-png.png" alt="X" style="width:7%; height:50%;">](https://x.com/BeatDropVis)
[<img src="https://static.vecteezy.com/system/resources/previews/018/930/718/non_2x/discord-logo-discord-icon-transparent-free-png.png" alt="Discord (Join server)" style="width:7%; height:50%;">](https://discord.gg/rp5cBDtGuM)

---------------------------------------------------------------------------------------------------------------------------------------------

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

## System Requirements
* Windows 11, Windows 10
* Minimum 2GB of RAM required
* WASAPI - compatible sound card
* DirectX 9 or higher - compatible GPU
* DirectX End-User [Runtimes](https://www.microsoft.com/en-us/download/details.aspx?id=8109) (also included in the installer) contains the required 32-bit helper libraries d3dx9_43.dll and d3dx9_31.dll

## Acknowledgements
Special thanks to:

* Ryan Geiss & Rovastar (John Baker) - [Official Milkdrop2 Source Code](https://sourceforge.net/projects/milkdrop2/)
* [oO-MrC-Oo](https://github.com/oO-MrC-Oo) - [XBMC Plug-in](https://github.com/oO-MrC-Oo/Milkdrop2-XBMC)
* Casey Langen ([@clangen](https://github.com/clangen)) - [milkdrop2-musikcube](https://github.com/clangen/milkdrop2-musikcube)
* Matthew van Eerde ([@mvaneerde](https://github.com/mvaneerde)) - [loopback-capture](https://github.com/mvaneerde/blog)
* projectM Team ([Kai Blaschke](https://github.com/kblaschke)) - [projectM-eval](https://github.com/projectM-visualizer/projectm-eval), contributor
* Lynn Jarvis ([@leadedge](https://github.com/leadedge)) - [BeatDrop for Spout](https://github.com/leadedge/BeatDrop), [Spout](https://spout.zeal.co), contributor
* Patrick Pomerleau @ Nest Immersion ([@nestdome](https://www.instagram.com/nestdome)) - help, contributor, feature integrations to NestDrop
* [IkeC](https://github.com/IkeC) - contributor, [Milkwave](https://github.com/IkeC/Milkwave) Developer
* ...and all the preset authors and contributors!

## License

[license]: #license

This repository is licensed under the 3-Clause BSD License ([LICENSE](LICENSE) or [https://opensource.org/licenses/BSD-3-Clause](https://opensource.org/licenses/BSD-3-Clause)) with the exception of where otherwise noted.

Although the original Matthew van Eerde's [loopback-capture](https://github.com/mvaneerde/blog) project didn't explicitly state the license, the author has been kind enough to provide the [license clarification](
https://blogs.msdn.microsoft.com/matthew_van_eerde/2014/11/05/draining-the-wasapi-capture-buffer-fully/)

> ### Sunny March 29, 2015 at 11:06 pm
> Hi. Just wondering is this open source? I'm looking for something like this for my school project.
>
> ### Maurits [MSFT] March 30, 2015 at 8:35 am
> @Sunny do with the source as you like.

All changes in this repository to the original Matthew's code are published either under the terms of BSD license or the license provided by original author.

## Contributions

Unless you explicitly state otherwise, any contribution intentionally submitted for inclusion in the work by you, shall be licensed as above.
