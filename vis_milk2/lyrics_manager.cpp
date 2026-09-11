#include "lyrics_manager.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iterator>
#include <sstream>

#include <windows.h>
#include <winhttp.h>

namespace
{
    constexpr int kNetworkTimeoutMs = 3000;

    std::string ReadWinHttpResponse(HINTERNET request)
    {
        std::string response;
        DWORD available = 0;
        while (WinHttpQueryDataAvailable(request, &available) && available)
        {
            std::string chunk(available, '\0');
            DWORD read = 0;
            if (!WinHttpReadData(request, &chunk[0], available, &read) || !read)
                break;
            response.append(chunk.data(), read);
        }
        return response;
    }

    bool HttpGet(const std::wstring& path, std::string& response)
    {
        // DEFAULT_PROXY is supported by Vista.  Avoid WinRT and automatic proxy
        // discovery so the OldOS build has the same bounded, background lookup.
        HINTERNET session = WinHttpOpen(L"BeatDrop Music Visualizer/1.5",
            WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME,
            WINHTTP_NO_PROXY_BYPASS, 0);
        if (!session)
            return false;

        WinHttpSetTimeouts(session, kNetworkTimeoutMs, kNetworkTimeoutMs,
            kNetworkTimeoutMs, kNetworkTimeoutMs);
        HINTERNET connection = WinHttpConnect(session, L"lrclib.net",
            INTERNET_DEFAULT_HTTPS_PORT, 0);
        HINTERNET request = connection ? WinHttpOpenRequest(connection, L"GET",
            path.c_str(), nullptr, WINHTTP_NO_REFERER,
            WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE) : nullptr;

        bool succeeded = request && WinHttpSendRequest(request,
            L"Accept: application/json\r\n", -1L, WINHTTP_NO_REQUEST_DATA, 0, 0, 0) &&
            WinHttpReceiveResponse(request, nullptr);
        if (succeeded)
        {
            DWORD status = 0;
            DWORD size = sizeof(status);
            succeeded = WinHttpQueryHeaders(request,
                WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
                WINHTTP_HEADER_NAME_BY_INDEX, &status, &size,
                WINHTTP_NO_HEADER_INDEX) && status >= 200 && status < 300;
            if (succeeded)
                response = ReadWinHttpResponse(request);
        }

        if (request) WinHttpCloseHandle(request);
        if (connection) WinHttpCloseHandle(connection);
        WinHttpCloseHandle(session);
        return succeeded;
    }

    bool ReadJsonString(const std::string& source, size_t from, const char* key,
        std::string& value, size_t* next = nullptr)
    {
        const std::string marker = std::string("\"") + key + "\"";
        const size_t keyAt = source.find(marker, from);
        if (keyAt == std::string::npos)
            return false;
        const size_t colon = source.find(':', keyAt + marker.size());
        if (colon == std::string::npos)
            return false;
        size_t at = colon + 1;
        while (at < source.size() && (source[at] == ' ' || source[at] == '\t' ||
            source[at] == '\r' || source[at] == '\n'))
            ++at;
        if (at >= source.size() || source[at] != '"')
            return false;

        value.clear();
        ++at;
        for (; at < source.size(); ++at)
        {
            const char ch = source[at];
            if (ch == '"')
            {
                if (next) *next = at + 1;
                return true;
            }
            if (ch != '\\' || ++at >= source.size())
            {
                value.push_back(ch);
                continue;
            }

            switch (source[at])
            {
            case 'b': value.push_back('\b'); break;
            case 'f': value.push_back('\f'); break;
            case 'n': value.push_back('\n'); break;
            case 'r': value.push_back('\r'); break;
            case 't': value.push_back('\t'); break;
            case '"': value.push_back('"'); break;
            case '\\': value.push_back('\\'); break;
            case '/': value.push_back('/'); break;
            default:
                // LRCLIB normally emits UTF-8 directly.  Preserve unfamiliar
                // escape sequences rather than corrupting an otherwise usable LRC.
                value.push_back(source[at]);
                break;
            }
        }
        return false;
    }

    bool FindSyncedLyrics(const std::string& response, std::wstring& rawLrc,
        std::vector<BeatDropLyricLine>& lines)
    {
        size_t cursor = 0;
        std::string synced;
        while (ReadJsonString(response, cursor, "syncedLyrics", synced, &cursor))
        {
            const std::wstring candidate = BeatDropLyricsManager::Wide(synced);
            std::vector<BeatDropLyricLine> parsed = BeatDropLyricsManager::ParseLrc(candidate);
            if (!parsed.empty())
            {
                rawLrc = candidate;
                lines.swap(parsed);
                return true;
            }
        }
        return false;
    }

    bool FindPlainLyrics(const std::string& response, std::wstring& plainLyrics)
    {
        size_t cursor = 0;
        std::string plain;
        while (ReadJsonString(response, cursor, "plainLyrics", plain, &cursor))
        {
            const std::wstring candidate = BeatDropLyricsManager::Wide(plain);
            if (!candidate.empty())
            {
                plainLyrics = candidate;
                return true;
            }
        }
        return false;
    }

    std::wstring MetadataValue(const std::wstring& value)
    {
        std::wstring result = value;
        for (wchar_t& ch : result)
        {
            if (ch == L'\r' || ch == L'\n')
                ch = L' ';
            else if (ch == L']')
                ch = L')';
        }
        return result;
    }

    std::wstring WithBeatDropMetadata(const std::wstring& lyrics,
        const std::wstring& artist, const std::wstring& title,
        const std::wstring& album, double durationSeconds)
    {
        const double safeDuration = durationSeconds > 0.0 ? durationSeconds : 0.0;
        size_t firstLyric = 0;
        while (firstLyric < lyrics.size() &&
            (lyrics[firstLyric] == L'\r' || lyrics[firstLyric] == L'\n'))
            ++firstLyric;
        std::wostringstream document;
        document << L"[duration:" << std::fixed << std::setprecision(2) << safeDuration << L"]\r\n"
                 << L"[ar:" << MetadataValue(artist) << L"]\r\n"
                 << L"[al:" << MetadataValue(album) << L"]\r\n"
                 << L"[ti:" << MetadataValue(title) << L"]\r\n"
                 << L"[length:" << std::fixed << std::setprecision(2) << safeDuration << L"]\r\n"
                 << L"[by:BeatDrop Music Visualizer]\r\n"
                 << L"[re:BeatDrop Music Visualizer]\r\n"
                 << L"[ve:1.0]\r\n"
                 << L"[offset:0]\r\n\r\n"
                 << lyrics.substr(firstLyric);
        return document.str();
    }

}

BeatDropLyricsManager::BeatDropLyricsManager()
    : m_worker(&BeatDropLyricsManager::Worker, this)
{
}

BeatDropLyricsManager::~BeatDropLyricsManager()
{
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_stop = true;
        m_requestPending = false;
    }
    m_condition.notify_one();
    if (m_worker.joinable())
        m_worker.join();
}

void BeatDropLyricsManager::UpdateTrack(const std::wstring& artist,
    const std::wstring& title, const std::wstring& album, double durationSeconds)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (artist.empty() && title.empty())
    {
        ++m_generation;
        m_requestPending = false;
        m_artist.clear();
        m_title.clear();
        m_album.clear();
        m_durationSeconds = 0.0;
        m_lines.clear();
        m_rawLrc.clear();
        m_status = L"No active now-playing track";
        return;
    }

    if (artist == m_artist && title == m_title && album == m_album)
    {
        m_durationSeconds = durationSeconds;
        return;
    }

    m_artist = artist;
    m_title = title;
    m_album = album;
    m_durationSeconds = durationSeconds;
    ++m_generation;
    m_requestPending = true;
    m_lines.clear();
    m_rawLrc.clear();
    m_status = L"Looking for synchronized lyrics";
    m_condition.notify_one();
}

std::wstring BeatDropLyricsManager::CurrentLine(double positionSeconds) const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_lines.empty() || positionSeconds < 0.0)
        return std::wstring();

    const auto firstAfterPosition = std::upper_bound(m_lines.begin(), m_lines.end(),
        positionSeconds, [](double position, const BeatDropLyricLine& line) {
            return position < line.startSeconds;
        });
    return firstAfterPosition == m_lines.begin() ? std::wstring() :
        (firstAfterPosition - 1)->text;
}

std::wstring BeatDropLyricsManager::CurrentLrc() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_rawLrc;
}

std::wstring BeatDropLyricsManager::Status() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_status;
}

void BeatDropLyricsManager::SetCurrentLrc(const std::wstring& text)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    // Invalidate a lookup that may be in flight for this track.  Its result
    // must not replace the LRC the user just imported into the editor.
    ++m_generation;
    m_requestPending = false;
    m_rawLrc = text;
    m_lines = ParseLrc(text);
    m_status = m_lines.empty() ? L"Imported LRC has no timed lines" :
        L"Imported synchronized lyrics ready";
}

std::wstring BeatDropLyricsManager::SaveCurrentLrc(const std::wstring& text,
    bool preserveHeader)
{
    std::wstring artist;
    std::wstring title;
    std::wstring album;
    double durationSeconds = 0.0;
    std::wstring document;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        artist = m_artist;
        title = m_title;
        album = m_album;
        durationSeconds = m_durationSeconds;
        document = preserveHeader ? text :
            WithBeatDropMetadata(text, artist, title, album, durationSeconds);
        m_rawLrc = document;
        m_lines = ParseLrc(m_rawLrc);
        m_status = m_lines.empty() ? L"Saved LRC has no timed lines" :
            L"Saved lyrics to the local cache";
    }
    if (artist.empty() && title.empty())
        return std::wstring();

    const std::wstring path = CachePath(artist, title);
    WriteUtf8File(path, document);
    return path;
}

void BeatDropLyricsManager::Refresh()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_artist.empty() && m_title.empty())
        return;
    ++m_generation;
    m_requestPending = true;
    m_status = L"Refreshing synchronized lyrics";
    m_condition.notify_one();
}

void BeatDropLyricsManager::Worker()
{
    for (;;)
    {
        std::wstring artist;
        std::wstring title;
        std::wstring album;
        double duration = 0.0;
        unsigned long long generation = 0;
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_condition.wait(lock, [this] { return m_stop || m_requestPending; });
            if (m_stop)
                return;
            m_requestPending = false;
            artist = m_artist;
            title = m_title;
            album = m_album;
            duration = m_durationSeconds;
            generation = m_generation;
        }

        const std::wstring cachePath = CachePath(artist, title);
        std::wstring rawLrc = ReadUtf8File(cachePath);
        std::vector<BeatDropLyricLine> lines = ParseLrc(rawLrc);
        // A non-empty cache with no timestamps is an editor-ready plain lyric,
        // not a cache miss.  It must not be replaced until the user timestamps
        // and explicitly saves it as a local LRC.
        if (rawLrc.empty())
        {
            FetchLyrics(artist, title, album, duration, lines, rawLrc);
            if (!rawLrc.empty())
            {
                // Background lookup caches the synchronized script exactly as
                // supplied. Metadata is added only by the explicit Save local
                // LRC action in the editor.
                WriteUtf8File(cachePath, rawLrc);
            }
        }

        std::lock_guard<std::mutex> lock(m_mutex);
        if (generation == m_generation)
        {
            m_lines.swap(lines);
            m_rawLrc.swap(rawLrc);
            m_status = m_rawLrc.empty() ? L"No lyrics found" :
                (m_lines.empty() ? L"Unsynchronized lyrics ready for editing" :
                    L"Synchronized lyrics ready");
        }
    }
}

bool BeatDropLyricsManager::FetchLyrics(const std::wstring& artist,
    const std::wstring& title, const std::wstring& album, double durationSeconds,
    std::vector<BeatDropLyricLine>& lines, std::wstring& rawLrc)
{
    // Some media sessions expose only a track title.  LRCLIB accepts an
    // artist-less search, so do not discard otherwise usable synchronized
    // lyrics merely because that optional field is unavailable.
    if (title.empty())
        return false;

    std::string response;
    std::wstring exactPath = L"/api/get?track_name=" + Wide(UrlEncode(title));
    if (!artist.empty())
        exactPath += L"&artist_name=" + Wide(UrlEncode(artist));
    if (!album.empty())
        exactPath += L"&album_name=" + Wide(UrlEncode(album));
    if (durationSeconds > 0.0)
        exactPath += L"&duration=" + std::to_wstring(static_cast<int>(durationSeconds + 0.5));
    std::wstring plainLyrics;
    if (HttpGet(exactPath, response))
    {
        if (FindSyncedLyrics(response, rawLrc, lines))
            return true;
        FindPlainLyrics(response, plainLyrics);
    }

    response.clear();
    std::wstring searchPath = L"/api/search?track_name=" + Wide(UrlEncode(title));
    if (!artist.empty())
        searchPath += L"&artist_name=" + Wide(UrlEncode(artist));
    if (HttpGet(searchPath, response))
    {
        if (FindSyncedLyrics(response, rawLrc, lines))
            return true;
        if (plainLyrics.empty())
            FindPlainLyrics(response, plainLyrics);
    }

    if (plainLyrics.empty())
        return false;
    rawLrc = plainLyrics;
    lines.clear();
    return true;
}

std::wstring BeatDropLyricsManager::CachePath(const std::wstring& artist,
    const std::wstring& title)
{
    wchar_t localAppData[MAX_PATH] = {};
    const DWORD length = GetEnvironmentVariableW(L"LOCALAPPDATA", localAppData, MAX_PATH);
    std::wstring root = length && length < MAX_PATH ?
        std::wstring(localAppData, length) : L".";
    root += L"\\BeatDrop";
    CreateDirectoryW(root.c_str(), nullptr);
    root += L"\\Lyrics";
    CreateDirectoryW(root.c_str(), nullptr);

    std::wstring readable = artist + L" - " + title;
    for (wchar_t& ch : readable)
        if (!(iswalnum(ch) || ch == L' ' || ch == L'-' || ch == L'_'))
            ch = L'_';
    if (readable.empty())
        readable = L"unknown";
    if (readable.size() > 96)
        readable.resize(96);

    return root + L"\\" + readable + L".lrc";
}

std::wstring BeatDropLyricsManager::ReadUtf8File(const std::wstring& path)
{
    std::ifstream file(path, std::ios::binary);
    if (!file)
        return std::wstring();
    std::string bytes((std::istreambuf_iterator<char>(file)),
        std::istreambuf_iterator<char>());
    if (bytes.size() >= 3 && static_cast<unsigned char>(bytes[0]) == 0xef &&
        static_cast<unsigned char>(bytes[1]) == 0xbb &&
        static_cast<unsigned char>(bytes[2]) == 0xbf)
        bytes.erase(0, 3);
    return Wide(bytes);
}

void BeatDropLyricsManager::WriteUtf8File(const std::wstring& path,
    const std::wstring& text)
{
    std::ofstream file(path, std::ios::binary | std::ios::trunc);
    if (!file)
        return;
    const std::string bytes = Utf8(text);
    file.write(bytes.data(), static_cast<std::streamsize>(bytes.size()));
}

std::wstring BeatDropLyricsManager::Wide(const std::string& text)
{
    if (text.empty())
        return std::wstring();
    const int count = MultiByteToWideChar(CP_UTF8, 0, text.data(),
        static_cast<int>(text.size()), nullptr, 0);
    if (count <= 0)
        return std::wstring();
    std::wstring result(static_cast<size_t>(count), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, text.data(), static_cast<int>(text.size()),
        &result[0], count);
    return result;
}

std::string BeatDropLyricsManager::Utf8(const std::wstring& text)
{
    if (text.empty())
        return std::string();
    const int count = WideCharToMultiByte(CP_UTF8, 0, text.data(),
        static_cast<int>(text.size()), nullptr, 0, nullptr, nullptr);
    if (count <= 0)
        return std::string();
    std::string result(static_cast<size_t>(count), '\0');
    WideCharToMultiByte(CP_UTF8, 0, text.data(), static_cast<int>(text.size()),
        &result[0], count, nullptr, nullptr);
    return result;
}

std::string BeatDropLyricsManager::UrlEncode(const std::wstring& text)
{
    const std::string input = Utf8(text);
    const char hex[] = "0123456789ABCDEF";
    std::string encoded;
    encoded.reserve(input.size() * 3);
    for (unsigned char ch : input)
    {
        if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') ||
            (ch >= '0' && ch <= '9') || ch == '-' || ch == '_' ||
            ch == '.' || ch == '~')
            encoded.push_back(static_cast<char>(ch));
        else
        {
            encoded.push_back('%');
            encoded.push_back(hex[ch >> 4]);
            encoded.push_back(hex[ch & 0x0f]);
        }
    }
    return encoded;
}
