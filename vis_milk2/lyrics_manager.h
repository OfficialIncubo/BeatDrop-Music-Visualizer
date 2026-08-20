#pragma once

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

struct BeatDropLyricLine
{
    double startSeconds = 0.0;
    std::wstring text;
};

class BeatDropLyricsManager
{
public:
    BeatDropLyricsManager();
    ~BeatDropLyricsManager();

    void UpdateTrack(const std::wstring& artist, const std::wstring& title,
                     const std::wstring& album, double durationSeconds);
    std::wstring CurrentLine(double positionSeconds) const;
    std::wstring CurrentLrc() const;
    std::wstring Status() const;
    void SaveCurrentLrc(const std::wstring& text);
    void Refresh();
    static std::vector<BeatDropLyricLine> ParseLrc(const std::wstring& text);
    static std::string UrlEncode(const std::wstring& text);
    static std::wstring Wide(const std::string& text);
    static std::string Utf8(const std::wstring& text);

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
    std::wstring m_status = L"Waiting for a Windows Now Playing track";
};
