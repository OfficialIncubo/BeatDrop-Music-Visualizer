#include <iostream>
#include <fstream>
#include <sstream>
#include <ctime>

#include <windows.h>

#include <direct.h>
#include <string>
#include <dbghelp.h>

#ifdef _WIN10_BUILD
#define SUPPORT_SMTC 1
#endif

#if SUPPORT_SMTC
#if NTDDI_VERSION >= NTDDI_WIN10
// Win RT
#include <winrt/Windows.Media.Control.h>
#include <winrt/Windows.Foundation.h>
#endif

using namespace winrt;
using namespace Windows::Media::Control;
using namespace std::chrono_literals;
#endif

class SongTitleGetter {

public:
	std::wstring currentArtist;
	std::wstring currentTitle;
	std::wstring currentAlbum;

	#if SUPPORT_SMTC
	std::chrono::steady_clock::time_point start_time;
	#endif

	bool updated = false;
	bool doPoll = true;
	bool doPollExplicit = false;
	bool SMTCSupported = false;
	bool isSongChange = false;

	SongTitleGetter();
	void Init();
	void PollMediaInfo();
};