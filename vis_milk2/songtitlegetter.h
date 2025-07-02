#include <iostream>
#include <fstream>
#include <sstream>
#include <ctime>

#include <windows.h>

#include <direct.h>
#include <string>
#include <dbghelp.h>

// Win RT
#if NTDDI_VERSION >= NTDDI_WIN10
#include <winrt/Windows.Media.Control.h>
#include <winrt/Windows.Foundation.h>
#endif

using namespace winrt;
using namespace Windows::Media::Control;
using namespace std::chrono_literals;

class SongTitleGetter {

public:
	std::wstring currentArtist;
	std::wstring currentTitle;
	std::wstring currentAlbum;

	std::chrono::steady_clock::time_point start_time;

	bool updated = false;
	bool doPoll = true;
	bool doPollExplicit = false;
	bool isSongChange = false;

	SongTitleGetter();
	void Init();
	void PollMediaInfo();
};