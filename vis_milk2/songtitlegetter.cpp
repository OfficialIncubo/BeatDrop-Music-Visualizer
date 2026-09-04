#include "songtitlegetter.h"

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
#include <algorithm>
#include <cstdint>

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
            try {
                const auto timeline = currentSession.GetTimelineProperties();
                currentPositionSeconds = std::chrono::duration<double>(timeline.Position()).count();
                currentDurationSeconds = std::chrono::duration<double>(timeline.EndTime() - timeline.StartTime()).count();
                if (currentPositionSeconds < 0.0) currentPositionSeconds = 0.0;
                if (currentDurationSeconds < 0.0) currentDurationSeconds = 0.0;
            }
            catch (const winrt::hresult_error&) {
                currentPositionSeconds = 0.0;
                currentDurationSeconds = 0.0;
            }

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
            currentPositionSeconds = 0.0;
            currentDurationSeconds = 0.0;
        }

        // Reset the start time to the current time
        start_time = current_time;
    }
#endif
}

bool SongTitleGetter::SeekTo(double positionSeconds)
{
#if SUPPORT_SMTC
    if (!SMTCSupported || positionSeconds < 0.0)
        return false;

    try {
        auto smtcManager = winrt::Windows::Media::Control::GlobalSystemMediaTransportControlsSessionManager::RequestAsync().get();
        auto currentSession = smtcManager.GetCurrentSession();
        if (!currentSession)
            return false;

        const auto timeline = currentSession.GetTimelineProperties();
        const double startSeconds = std::chrono::duration<double>(timeline.StartTime()).count();
        const double endSeconds = std::chrono::duration<double>(timeline.EndTime()).count();
        double targetSeconds = positionSeconds + startSeconds;
        if (endSeconds > startSeconds)
            targetSeconds = (std::min)(targetSeconds, endSeconds);
        if (targetSeconds < startSeconds)
            targetSeconds = startSeconds;

        const auto target = std::chrono::duration_cast<std::chrono::duration<int64_t, std::ratio<1, 10000000>>>(
            std::chrono::duration<double>(targetSeconds));
        currentSession.TryChangePlaybackPositionAsync(target.count()).get();
        currentPositionSeconds = positionSeconds;
        return true;
    }
    catch (const winrt::hresult_error&) {
        return false;
    }
#else
    (void)positionSeconds;
    return false;
#endif
}

bool SongTitleGetter::SeekRelative(double deltaSeconds)
{
    return SeekTo((std::max)(0.0, currentPositionSeconds + deltaSeconds));
}
