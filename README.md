# Milkwave

Milkwave is a music visualizer based on the [MilkDrop2](https://www.geisswerks.com/milkdrop/) visualizer. 

Milkwave's main new feature is a standalone window called **Milkwave Remote** which allows you to send customized messages to display in the visualizer, either directly by typing or from a script file. You may also load and change presets or send common key combinations using buttons, all without ever touching the visualizer window.

![image](https://github.com/user-attachments/assets/9a771f33-1ef2-4188-877f-f442aa765799)

## Features

* Customize message text, color, font size, position and other parameters
* Save parameter definitions as named styles for quick access
* Send messages, set parameters, presets etc. automatically from a script file at configurable intervals
* Select preset files and send them to the visualizer
* Show the currently playing visualizer preset in the Remote window
* Send common key combinations using buttons

There are many tooltips explaining all features when you hover over the form elements.

## History

The original Milkdrop 2 WinAMP plugin created by Ryan Geiss was turned into a Windows standalone application by Maxim Volskiy as [BeatDrop](https://github.com/mvsoft74/BeatDrop) and been improved upon eg. in the [BeatDrop-Music-Visualizer](https://github.com/OfficialIncubo/BeatDrop-Music-Visualizer) and [MilkDrop3](https://github.com/milkdrop2077/MilkDrop3) projects.

## System Requirements
* Windows 7 or later
* DirectX 9 or higher - compatible GPU
* [Microsoft .NET Desktop Runtime 8](https://dotnet.microsoft.com/en-us/download/dotnet/8.0)
* DirectX End-User [Runtimes](https://www.microsoft.com/en-us/download/details.aspx?id=8109) (also included in the installer) contains the required 32-bit helper libraries d3dx9_43.dll and d3dx9_31.dll

## Support

I may add new features or fix bugs if anyone is actually using this, so feel free to open an issue or join my [**Discord**](https://bit.ly/Ikes-Discord)! 

This project incorporates the work of many different authors over the years, as listed below. Naturally, the entirety of this project is open source and there will never be a paid version of it.

However, if you're satisfied with the additions I made and you want to support my work, here's my [**PayPal account**](https://www.paypal.com/ncp/payment/5XMP3S69PJLCU). You'll be listed as a supporter within the next program release if you wish.

## Acknowledgements

Many thanks to:

* Ryan Geiss - [Milkdrop2](https://www.geisswerks.com/milkdrop/)
* Maxim Volskiy - [BeatDrop](https://github.com/mvsoft74/BeatDrop)
* oO-MrC-Oo - [XBMC plugin](https://github.com/oO-MrC-Oo/Milkdrop2-XBMC)
* Casey Langen - [milkdrop2-musikcube](https://github.com/clangen/milkdrop2-musikcube)
* Matthew van Eerde - [loopback-capture](https://github.com/mvaneerde/blog)
* Incubo_ - [BeatDrop-Music-Visualizer](https://github.com/OfficialIncubo/BeatDrop-Music-Visualizer)
* milkdrop2077 - [MilkDrop3](https://github.com/milkdrop2077/MilkDrop3)
* podenthusiast - [Milkwave Logo](https://www.freepik.com/author/podenthusiast/icons)
* and all the preset authors!

If you believe you should be mentioned here, please let me know.

## License

[license]: #license

Milkdrop Remote is licensed under the [Attribution-NonCommercial 4.0 International License](https://creativecommons.org/licenses/by-nc/4.0/), meaning you may not use the material for commercial purposes. See LICENSE.txt for details.

Milkdrop Visualizer (as a BeatDrop fork) is licensed under the [3-Clause BSD License](https://opensource.org/licenses/BSD-3-Clause) with the exception of where otherwise noted. See LICENSE.txt for details.

Although the original Matthew van Eerde's [loopback-capture](https://github.com/mvaneerde/blog) project didn't explicitly state the license, the author has been kind enough to provide a license clarification [here](
https://blogs.msdn.microsoft.com/matthew_van_eerde/2014/11/05/draining-the-wasapi-capture-buffer-fully/). All changes in this repository to the original Matthew's code are published either under the terms of BSD license or the license provided by original author.

## Contributions

Unless you explicitly state otherwise, any contribution intentionally submitted for inclusion in the work by you, shall be licensed as above.
