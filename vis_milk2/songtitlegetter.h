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
	double currentDurationSeconds = 0.0;
	double currentPositionSeconds = 0.0;

	#if SUPPORT_SMTC
	std::chrono::steady_clock::time_point start_time;
	std::chrono::steady_clock::time_point timeline_sample_time;
	double timeline_sample_position = 0.0;
	bool timeline_sample_valid = false;
	bool timeline_was_playing = false;
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
	bool SeekRelative(double seconds);
	bool SeekTo(double seconds);
};
