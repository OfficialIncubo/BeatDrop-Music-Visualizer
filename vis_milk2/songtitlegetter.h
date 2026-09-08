#include <iostream>
#include <fstream>
#include <sstream>
#include <ctime>

#include <windows.h>

#include <direct.h>
#include <string>
#include <chrono>
#include <cstdint>
#include <dbghelp.h>
#include "songtimeline.h"

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
	SongTimelineClock timeline_clock;
	GlobalSystemMediaTransportControlsSessionManager smtc_manager{ nullptr };
	GlobalSystemMediaTransportControlsSession timeline_session{ nullptr };
	std::chrono::steady_clock::time_point last_manager_attempt;
	std::chrono::steady_clock::time_point last_metadata_poll;
	bool has_manager_attempt = false;
	bool has_metadata_poll = false;
	#endif

	bool updated = false;
	bool doPoll = true;
    bool doPollExplicit = false;
    bool SMTCSupported = false;
    bool isSongChange = false;
    bool winrtInitialized = false;

	SongTitleGetter();
	void Init();
	void PollMediaInfo();
	int64_t GetPositionMilliseconds() const;
	int64_t GetDurationMilliseconds() const;
	bool EnsureMediaManager();
};