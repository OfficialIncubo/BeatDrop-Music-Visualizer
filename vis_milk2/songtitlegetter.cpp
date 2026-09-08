#include "songtitlegetter.h"

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
    timeline_clock.Reset();
#else
    SMTCSupported = false;
#endif
}

void SongTitleGetter::PollMediaInfo() {

    if (!SMTCSupported) return;
    if (!doPoll && !doPollExplicit) return;

    #if SUPPORT_SMTC

    try {
        auto smtcManager = winrt::Windows::Media::Control::GlobalSystemMediaTransportControlsSessionManager::RequestAsync().get();
        auto currentSession = smtcManager.GetCurrentSession();
        updated = false;
        if (currentSession) {
            const bool sessionChanged = timeline_session != currentSession;
            timeline_session = currentSession;
            auto properties = currentSession.TryGetMediaPropertiesAsync().get();
            bool trackChanged = sessionChanged;
            if (properties) {
                if (properties.Artist().c_str() != currentArtist || properties.Title().c_str() != currentTitle ||
                    properties.AlbumTitle().c_str() != currentAlbum) {
                    trackChanged = true;
                    isSongChange = currentArtist.length() || currentTitle.length();
                    currentArtist = properties.Artist().c_str();
                    currentTitle = properties.Title().c_str();
                    currentAlbum = properties.AlbumTitle().c_str();

                    updated = true;
                }
            }

            const auto timeline = currentSession.GetTimelineProperties();
            const auto playback = currentSession.GetPlaybackInfo();
            const bool reportedPlaying = playback.PlaybackStatus() ==
                GlobalSystemMediaTransportControlsSessionPlaybackStatus::Playing;
            const auto rate = playback.PlaybackRate();
            if (trackChanged)
                timeline_clock.Reset();

            const int64_t start = timeline.StartTime().count() / 10000;
            timeline_clock.Update(
                timeline.Position().count() / 10000 - start,
                timeline.EndTime().count() / 10000 - start,
                timeline.LastUpdatedTime().time_since_epoch().count() / 10000,
                winrt::clock::now().time_since_epoch().count() / 10000,
                std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::steady_clock::now().time_since_epoch()).count(),
                reportedPlaying, rate ? rate.Value() : 1.0);
        }
        else {
            timeline_clock.Reset();
            timeline_session = nullptr;
            if (currentArtist.length() || currentTitle.length()) {
                currentArtist = L"";
                currentTitle = L"";
                currentAlbum = L"";
                updated = true;
            }
        }
    }
    catch (const winrt::hresult_error&) {
        // A media player can close or replace its SMTC session between calls.
        // WinRT objects are automatic here; clearing this state keeps the
        // renderer safe and avoids showing stale time data.
        timeline_clock.Reset();
        timeline_session = nullptr;
    }
#endif
}

int64_t SongTitleGetter::GetPositionMilliseconds() const
{
#if SUPPORT_SMTC
    return timeline_clock.Position(std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count());
#else
    return -1;
#endif
}

int64_t SongTitleGetter::GetDurationMilliseconds() const
{
#if SUPPORT_SMTC
    return timeline_clock.Duration();
#else
    return -1;
#endif
}