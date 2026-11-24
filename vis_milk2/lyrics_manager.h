#pragma once

#include "lyrics_lrc.h"

#include <condition_variable>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

// Owns one current-track LRC document and fetches/caches it off the render
// thread.  All public readers are thread-safe and never perform I/O.
class BeatDropLyricsManager
{
public:
    BeatDropLyricsManager();
    ~BeatDropLyricsManager();

    BeatDropLyricsManager(const BeatDropLyricsManager&) = delete;
    BeatDropLyricsManager& operator=(const BeatDropLyricsManager&) = delete;

    void UpdateTrack(const std::wstring& artist, const std::wstring& title,
        const std::wstring& album, double durationSeconds);
    std::wstring CurrentLine(double positionSeconds) const;
    std::wstring CurrentLrc() const;
    std::wstring Status() const;
    // Applies editor-imported lyrics immediately without writing a cache file.
    // This makes imported timestamps available to the live renderer at once.
    void SetCurrentLrc(const std::wstring& text);
    // Returns the UTF-8 LRC path when the current track has cache metadata.
    // Keeping this visible lets the editor tell the user exactly where a local
    // save landed without duplicating the cache naming rules.
    // When the editor supplies an existing metadata block, preserve it exactly
    // instead of regenerating BeatDrop's default header.
    std::wstring SaveCurrentLrc(const std::wstring& text, bool preserveHeader = false);
    // Deletes only cached .lrc files from BeatDrop's dedicated local cache.
    // The active in-memory lyric remains available until the track changes.
    int ClearLocalCache();
    void Refresh();

    static std::vector<BeatDropLyricLine> ParseLrc(const std::wstring& text)
    {
        return BeatDropLrc::Parse(text);
    }
    static std::wstring Wide(const std::string& text);
    static std::string Utf8(const std::wstring& text);
    static std::string UrlEncode(const std::wstring& text);

private:
    void Worker();
    static std::wstring CachePath(const std::wstring& artist, const std::wstring& title);
    static std::wstring ReadUtf8File(const std::wstring& path);
    static void WriteUtf8File(const std::wstring& path, const std::wstring& text);
    static bool FetchLyrics(const std::wstring& artist, const std::wstring& title,
        const std::wstring& album, double durationSeconds,
        std::vector<BeatDropLyricLine>& lines, std::wstring& rawLrc);
    mutable std::mutex m_mutex;
    std::condition_variable m_condition;
    std::thread m_worker;
    bool m_stop = false;
    bool m_requestPending = false;
    unsigned long long m_generation = 0;
    std::wstring m_artist;
    std::wstring m_title;
    std::wstring m_album;
    double m_durationSeconds = 0.0;
    std::vector<BeatDropLyricLine> m_lines;
    std::wstring m_rawLrc;
    std::wstring m_status = L"Waiting for a now-playing track";
};
