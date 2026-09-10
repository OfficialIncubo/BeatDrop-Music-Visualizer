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

bool SongTitleGetter::EnsureMediaManager()
{
#if SUPPORT_SMTC
    if (smtc_manager)
        return true;

    const auto now = std::chrono::steady_clock::now();
    constexpr auto retryInterval = std::chrono::seconds(2);
    if (has_manager_attempt && now - last_manager_attempt < retryInterval)
        return false;

    has_manager_attempt = true;
    last_manager_attempt = now;
    try {
        smtc_manager = GlobalSystemMediaTransportControlsSessionManager::RequestAsync().get();
        return static_cast<bool>(smtc_manager);
    }
    catch (const winrt::hresult_error&) {
        return false;
    }
#else
    return false;
#endif
}

void SongTitleGetter::PollMediaInfo() {

    if (!SMTCSupported) return;
    if (!doPoll && !doPollExplicit) return;

    #if SUPPORT_SMTC

    if (!EnsureMediaManager()) return;

    try {
        auto currentSession = smtc_manager.GetCurrentSession();
        updated = false;
        if (currentSession) {
            const bool sessionChanged = timeline_session != currentSession;
            timeline_session = currentSession;
            bool trackChanged = sessionChanged;

            const auto now = std::chrono::steady_clock::now();
            constexpr auto metadataPollInterval = std::chrono::seconds(1);
            if (sessionChanged || doPollExplicit || !has_metadata_poll ||
                now - last_metadata_poll >= metadataPollInterval) {
                auto properties = currentSession.TryGetMediaPropertiesAsync().get();
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
                has_metadata_poll = true;
                last_metadata_poll = now;
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
			has_metadata_poll = false;
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
		has_metadata_poll = false;
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

bool SongTitleGetter::SeekTo(double positionSeconds)
{
#if SUPPORT_SMTC
    if (!SMTCSupported || !std::isfinite(positionSeconds) || positionSeconds < 0.0 ||
        !EnsureMediaManager())
        return false;

    try {
        auto session = smtc_manager.GetCurrentSession();
        if (!session)
            return false;

        const auto timeline = session.GetTimelineProperties();
        const int64_t start = timeline.StartTime().count() / 10000;
        const int64_t end = timeline.EndTime().count() / 10000;
        int64_t target = start + static_cast<int64_t>(positionSeconds * 1000.0 + 0.5);
        if (end > start)
            target = (std::min)(target, end);
        target = (std::max)(target, start);

        // SMTC expects a 100-nanosecond TimeSpan count, not a chrono duration.
        if (!session.TryChangePlaybackPositionAsync(target * 10000).get())
            return false;

        // The next provider publication may lag the seek.  Anchor the shared
        // clock immediately so the renderer and editor stay on the selected line.
        const auto playback = session.GetPlaybackInfo();
        const bool playing = playback.PlaybackStatus() ==
            GlobalSystemMediaTransportControlsSessionPlaybackStatus::Playing;
        const auto rate = playback.PlaybackRate();
        const int64_t nowSteady = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count();
        const int64_t nowUtc = winrt::clock::now().time_since_epoch().count() / 10000;
        timeline_clock.Update(target - start, end - start, nowUtc, nowUtc, nowSteady,
            playing, rate ? rate.Value() : 1.0);
        timeline_session = session;
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
    const int64_t position = GetPositionMilliseconds();
    if (position < 0 || !std::isfinite(deltaSeconds))
        return false;
    return SeekTo((std::max)(0.0, position / 1000.0 + deltaSeconds));
}
