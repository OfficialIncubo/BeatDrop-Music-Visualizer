#include "songtitlegetter.h"

#include <fstream>

namespace
{
    void TraceTimeline(const std::wstring& artist, const std::wstring& title,
                       double position, double duration)
    {
        wchar_t temp[MAX_PATH] = {};
        const DWORD length = GetTempPathW(MAX_PATH, temp);
        if (!length || length >= MAX_PATH) return;
        std::wofstream file(std::wstring(temp, length) + L"BeatDropSMTC.log", std::ios::app);
        file << L"artist=" << artist << L" title=" << title
             << L" position=" << position << L" duration=" << duration << L"\n";
    }
}

SongTitleGetter::SongTitleGetter()
{
#if SUPPORT_SMTC
    SMTCSupported = true;
#else
    SMTCSupported = false;
#endif
}

void SongTitleGetter::Init() {
#if SUPPORT_SMTC
    if (!SMTCSupported || winrtInitialized)
        return;

    try {
        // The render thread is an MTA. Initializing this once prevents the
        // first few startup frames from repeatedly increasing its WinRT/COM
        // initialization count before MediaTexture creates a WIC GIF decoder.
        winrt::init_apartment(winrt::apartment_type::multi_threaded);
        winrtInitialized = true;
    }
    catch (const winrt::hresult_error&) {
        SMTCSupported = false;
        return;
    }
    start_time = std::chrono::steady_clock::now();
#else
    SMTCSupported = false;
#endif
}

void SongTitleGetter::PollMediaInfo() {

    if (!SMTCSupported) return;
    if (!doPoll && !doPollExplicit) return;

    #if SUPPORT_SMTC

    // Get the current time
    auto current_time = std::chrono::steady_clock::now();

    // Calculate the elapsed time in seconds
    auto elapsed_seconds = std::chrono::duration_cast<std::chrono::seconds>(current_time - start_time).count();

    if (elapsed_seconds >= 0 || doPollExplicit) {

        auto smtcManager = winrt::Windows::Media::Control::GlobalSystemMediaTransportControlsSessionManager::RequestAsync().get();
        auto currentSession = smtcManager.GetCurrentSession();
        updated = false;
        if (currentSession) {
            auto properties = currentSession.TryGetMediaPropertiesAsync().get();
            if (properties) {
                auto timeline = currentSession.GetTimelineProperties();
                const double durationSeconds = timeline.EndTime().count() / 10000000.0;
                const double reportedPositionSeconds = timeline.Position().count() / 10000000.0;
                const auto playbackInfo = currentSession.GetPlaybackInfo();
                const bool isPlaying = playbackInfo.PlaybackStatus() ==
                    GlobalSystemMediaTransportControlsSessionPlaybackStatus::Playing;
                const auto now = std::chrono::steady_clock::now();
                const bool trackChanged = properties.Artist().c_str() != currentArtist ||
                    properties.Title().c_str() != currentTitle ||
                    properties.AlbumTitle().c_str() != currentAlbum;
                if (doPollExplicit || trackChanged) {
                    isSongChange = currentArtist.length() || currentTitle.length();
                    currentArtist = properties.Artist().c_str();
                    currentTitle = properties.Title().c_str();
					currentAlbum = properties.AlbumTitle().c_str();

                    updated = true;
                }
				if (trackChanged || !timeline_sample_valid || isPlaying != timeline_was_playing ||
					std::abs(reportedPositionSeconds - timeline_sample_position) > 1.0)
				{
					timeline_sample_position = reportedPositionSeconds;
					timeline_sample_time = now;
					timeline_sample_valid = true;
				}
				double positionSeconds = timeline_sample_position;
				if (isPlaying)
					positionSeconds += std::chrono::duration<double>(now - timeline_sample_time).count();
				if (durationSeconds > 0.0)
					positionSeconds = (std::min)(positionSeconds, durationSeconds);
				currentPositionSeconds = (std::max)(0.0, positionSeconds);
				timeline_was_playing = isPlaying;
				currentDurationSeconds = durationSeconds;
				static double lastTracePosition = -100.0;
				if (doPollExplicit || std::abs(positionSeconds - lastTracePosition) >= 1.0 || updated)
				{
					TraceTimeline(currentArtist, currentTitle, positionSeconds, durationSeconds);
					lastTracePosition = positionSeconds;
				}
            }
        }
        else {
            if (currentArtist.length() || currentTitle.length()) {
                currentArtist = L"";
                currentTitle = L"";
                updated = true;
            }
        }

        // Reset the start time to the current time
        start_time = current_time;
    }
#endif
}

bool SongTitleGetter::SeekRelative(double seconds)
{
    return SeekTo(currentPositionSeconds + seconds);
}

bool SongTitleGetter::SeekTo(double seconds)
{
#if SUPPORT_SMTC
    if (!SMTCSupported) return false;
    bool initializedHere = false;
    try
    {
        try
        {
            winrt::init_apartment(winrt::apartment_type::multi_threaded);
            initializedHere = true;
        }
        catch (const winrt::hresult_error& error)
        {
            if (error.code() != RPC_E_CHANGED_MODE) throw;
        }

        auto manager = winrt::Windows::Media::Control::GlobalSystemMediaTransportControlsSessionManager::RequestAsync().get();
        auto session = manager.GetCurrentSession();
        if (!session) { if (initializedHere) winrt::uninit_apartment(); return false; }
        const auto timeline = session.GetTimelineProperties();
        const double duration = timeline.EndTime().count() / 10000000.0;
        const double target = (std::max)(0.0, duration > 0.0 ? (std::min)(seconds, duration) : seconds);
        const bool changed = session.TryChangePlaybackPositionAsync(
            static_cast<int64_t>(target * 10000000.0)).get();
        currentPositionSeconds = target;
        doPollExplicit = true;
        if (initializedHere) winrt::uninit_apartment();
        return changed;
    }
    catch (...)
    {
        if (initializedHere) winrt::uninit_apartment();
        return false;
    }
#else
    (void)seconds;
    return false;
#endif
}
