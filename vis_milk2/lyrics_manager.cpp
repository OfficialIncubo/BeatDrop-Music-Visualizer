#include "lyrics_manager.h"

#include <algorithm>
#include <cmath>
#include <cwctype>
#include <fstream>
#include <sstream>

#include <windows.h>
#include <winhttp.h>

#if defined(_WIN10_BUILD)
#include <winrt/Windows.Data.Json.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Web.Http.h>
#include <winrt/Windows.Web.Http.Headers.h>
#endif

namespace
{
    void LyricsTrace(const std::string& message)
    {
        wchar_t temp[MAX_PATH] = {};
        const DWORD length = GetTempPathW(MAX_PATH, temp);
        if (!length || length >= MAX_PATH) return;
        std::ofstream file(std::wstring(temp, length) + L"BeatDropLyrics.log", std::ios::app);
        file << message << "\n";
    }

    std::string ReadWinHttpResponse(HINTERNET request)
    {
        std::string body;
        DWORD available = 0;
        while (WinHttpQueryDataAvailable(request, &available) && available)
        {
            std::string chunk(available, '\0');
            DWORD read = 0;
            if (!WinHttpReadData(request, chunk.data(), available, &read) || read == 0)
                break;
            body.append(chunk.data(), read);
        }
        return body;
    }

    std::string JsonStringValue(const std::string& body, const char* key)
    {
        const std::string marker = std::string("\"") + key + "\"";
        const size_t keyAt = body.find(marker);
        if (keyAt == std::string::npos) return {};
        size_t at = body.find(':', keyAt + marker.size());
        at = at == std::string::npos ? at : body.find('"', at + 1);
        if (at == std::string::npos) return {};
        std::string value;
        for (++at; at < body.size(); ++at)
        {
            if (body[at] == '"') break;
            if (body[at] == '\\' && at + 1 < body.size())
            {
                const char escaped = body[++at];
                value.push_back(escaped == 'n' ? '\n' : escaped == 'r' ? '\r' : escaped == 't' ? '\t' : escaped);
            }
            else value.push_back(body[at]);
        }
        return value;
    }

    bool FetchLyricsWinHttp(const std::wstring& artist, const std::wstring& title,
                            const std::wstring& album, double durationSeconds,
                            std::vector<BeatDropLyricLine>& lines, std::wstring& rawLrc)
    {
        LyricsTrace("lookup " + BeatDropLyricsManager::Utf8(artist) + " / " + BeatDropLyricsManager::Utf8(title));
        HINTERNET session = WinHttpOpen(L"BeatDrop Music Visualizer/1.0", WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY, nullptr, nullptr, 0);
        if (!session) { LyricsTrace("WinHttpOpen failed " + std::to_string(GetLastError())); return false; }
        HINTERNET connection = WinHttpConnect(session, L"lrclib.net", INTERNET_DEFAULT_HTTPS_PORT, 0);
        if (!connection) { LyricsTrace("WinHttpConnect failed " + std::to_string(GetLastError())); WinHttpCloseHandle(session); return false; }

        (void)album; (void)durationSeconds;
        const std::wstring path = L"/api/search?track_name=" +
            BeatDropLyricsManager::Wide(BeatDropLyricsManager::UrlEncode(title)) +
            L"&artist_name=" + BeatDropLyricsManager::Wide(BeatDropLyricsManager::UrlEncode(artist));
        HINTERNET request = WinHttpOpenRequest(connection, L"GET", path.c_str(), nullptr,
            WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
        bool ok = request && WinHttpSendRequest(request, WINHTTP_NO_ADDITIONAL_HEADERS, 0, nullptr, 0, 0, 0) &&
            WinHttpReceiveResponse(request, nullptr);
        std::string body = ok ? ReadWinHttpResponse(request) : std::string();
        if (request) WinHttpCloseHandle(request);
        WinHttpCloseHandle(connection); WinHttpCloseHandle(session);
        LyricsTrace("http ok=" + std::to_string(ok ? 1 : 0) + " body=" + std::to_string(body.size()));
        if (!ok) { LyricsTrace("request failed " + std::to_string(GetLastError())); return false; }

        size_t cursor = 0;
        while ((cursor = body.find("\"syncedLyrics\"", cursor)) != std::string::npos)
        {
            const std::string synced = JsonStringValue(body.substr(cursor), "syncedLyrics");
            if (!synced.empty())
            {
                const std::wstring wide = BeatDropLyricsManager::Wide(synced);
                auto parsed = BeatDropLyricsManager::ParseLrc(wide);
                if (!parsed.empty()) { LyricsTrace("synced lyrics parsed"); lines = std::move(parsed); rawLrc = wide; return true; }
            }
            cursor += 15;
        }
        LyricsTrace("no synced lyrics field parsed");
        return false;
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

void BeatDropLyricsManager::UpdateTrack(const std::wstring& artist, const std::wstring& title,
                                        const std::wstring& album, double durationSeconds)
{
    if (artist.empty() && title.empty())
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        ++m_generation;
        m_requestPending = false;
        m_artist.clear();
        m_title.clear();
        m_album.clear();
        m_lines.clear();
        m_rawLrc.clear();
        m_status = L"No active Windows Now Playing track";
        return;
    }

    std::lock_guard<std::mutex> lock(m_mutex);
    if (artist == m_artist && title == m_title && album == m_album &&
        std::abs(durationSeconds - m_durationSeconds) < 1.0)
        return;

    m_artist = artist;
    m_title = title;
    m_album = album;
    m_durationSeconds = durationSeconds;
    ++m_generation;
    m_requestPending = true;
    m_lines.clear();
    m_rawLrc.clear();
    m_status = L"Looking for cached lyrics";
    m_condition.notify_one();
}

std::wstring BeatDropLyricsManager::CurrentLine(double positionSeconds) const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_lines.empty())
        return std::wstring();

    const BeatDropLyricLine* active = nullptr;
    for (const auto& line : m_lines)
    {
        if (line.startSeconds > positionSeconds)
            break;
        active = &line;
    }
    return active ? active->text : std::wstring();
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

void BeatDropLyricsManager::Refresh()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_artist.empty() || m_title.empty()) return;
    m_requestPending = true;
    m_status = L"Querying LRCLIB";
    m_condition.notify_one();
}

void BeatDropLyricsManager::SaveCurrentLrc(const std::wstring& text)
{
    std::wstring artist, title;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        artist = m_artist;
        title = m_title;
        m_rawLrc = text;
        m_lines = ParseLrc(text);
    }
    if (!artist.empty() && !title.empty())
    {
        WriteUtf8File(CachePath(artist, title), text);
        std::lock_guard<std::mutex> lock(m_mutex);
        m_status = L"Saved lyrics to local cache";
    }
}

void BeatDropLyricsManager::Worker()
{
#if defined(_WIN10_BUILD)
    try { winrt::init_apartment(winrt::apartment_type::multi_threaded); }
    catch (...) { }
#endif
    for (;;)
    {
        std::wstring artist, title, album;
        double duration = 0.0;
        unsigned long long generation = 0;
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_condition.wait(lock, [this] { return m_stop || m_requestPending; });
            if (m_stop)
            {
#if defined(_WIN10_BUILD)
                winrt::uninit_apartment();
#endif
                return;
            }
            m_requestPending = false;
            artist = m_artist;
            title = m_title;
            album = m_album;
            duration = m_durationSeconds;
            generation = m_generation;
        }

        std::vector<BeatDropLyricLine> lines;
        std::wstring rawLrc = ReadUtf8File(CachePath(artist, title));
        if (!rawLrc.empty())
            lines = ParseLrc(rawLrc);
        if (lines.empty())
        {
            FetchLyrics(artist, title, album, duration, lines, rawLrc);
            // Keep the first successful network result so the next playback
            // starts locally and the editor can open it immediately.
            if (!lines.empty() && !rawLrc.empty())
                WriteUtf8File(CachePath(artist, title), rawLrc);
        }

        std::lock_guard<std::mutex> lock(m_mutex);
        if (generation == m_generation)
        {
            m_lines = std::move(lines);
            m_rawLrc = std::move(rawLrc);
            m_status = m_lines.empty() ? L"No synchronized lyrics found" : L"Lyrics ready";
        }
    }
}

std::vector<BeatDropLyricLine> BeatDropLyricsManager::ParseLrc(const std::wstring& text)
{
    std::vector<BeatDropLyricLine> result;
    std::wistringstream input(text);
    std::wstring line;
    while (std::getline(input, line))
    {
        size_t lastClose = line.find_last_of(L']');
        if (lastClose == std::wstring::npos)
            continue;

        const std::wstring content = line.substr(lastClose + 1);
        size_t cursor = 0;
        while ((cursor = line.find(L'[', cursor)) != std::wstring::npos && cursor < lastClose)
        {
            const size_t colon = line.find(L':', cursor + 1);
            const size_t close = line.find(L']', cursor + 1);
            if (colon == std::wstring::npos || close == std::wstring::npos || colon > close)
                break;

            try
            {
                const int minutes = std::stoi(line.substr(cursor + 1, colon - cursor - 1));
                const double seconds = std::stod(line.substr(colon + 1, close - colon - 1));
                std::wstring trimmed = content;
                const size_t first = trimmed.find_first_not_of(L" \t");
                if (first != std::wstring::npos)
                    trimmed.erase(0, first);
                if (!trimmed.empty())
                    result.push_back({minutes * 60.0 + seconds, trimmed});
            }
            catch (...) { }
            cursor = close + 1;
        }
    }

    std::sort(result.begin(), result.end(), [](const auto& a, const auto& b) {
        return a.startSeconds < b.startSeconds;
    });
    return result;
}

std::string BeatDropLyricsManager::Utf8(const std::wstring& text)
{
    if (text.empty()) return std::string();
    const int size = WideCharToMultiByte(CP_UTF8, 0, text.c_str(), (int)text.size(), nullptr, 0, nullptr, nullptr);
    std::string result(size, '\0');
    WideCharToMultiByte(CP_UTF8, 0, text.c_str(), (int)text.size(), result.data(), size, nullptr, nullptr);
    return result;
}

std::wstring BeatDropLyricsManager::Wide(const std::string& text)
{
    if (text.empty()) return std::wstring();
    const int size = MultiByteToWideChar(CP_UTF8, 0, text.c_str(), (int)text.size(), nullptr, 0);
    std::wstring result(size, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, text.c_str(), (int)text.size(), result.data(), size);
    return result;
}

std::string BeatDropLyricsManager::UrlEncode(const std::wstring& text)
{
    const std::string input = Utf8(text);
    const char hex[] = "0123456789ABCDEF";
    std::string result;
    for (unsigned char c : input)
    {
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
            (c >= '0' && c <= '9') || c == '-' || c == '_' || c == '.' || c == '~')
            result.push_back((char)c);
        else
        {
            result.push_back('%');
            result.push_back(hex[c >> 4]);
            result.push_back(hex[c & 15]);
        }
    }
    return result;
}

std::wstring BeatDropLyricsManager::CachePath(const std::wstring& artist, const std::wstring& title)
{
    wchar_t localAppData[MAX_PATH] = {0};
    DWORD length = GetEnvironmentVariableW(L"LOCALAPPDATA", localAppData, MAX_PATH);
    std::wstring root = length ? std::wstring(localAppData, length) : L".";
    root += L"\\BeatDrop\\Lyrics";
    CreateDirectoryW((root.substr(0, root.rfind(L'\\'))).c_str(), nullptr);
    CreateDirectoryW(root.c_str(), nullptr);

    std::wstring name = artist + L" - " + title;
    for (wchar_t& c : name)
        if (!iswalnum(c) && c != L' ' && c != L'-' && c != L'_') c = L'_';
    while (!name.empty() && name.back() == L' ') name.pop_back();
    if (name.empty()) name = L"unknown";
    if (name.size() > 180) name.resize(180);
    return root + L"\\" + name + L".lrc";
}

std::wstring BeatDropLyricsManager::ReadUtf8File(const std::wstring& path)
{
    std::ifstream file(path, std::ios::binary);
    if (!file) return std::wstring();
    std::string bytes((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    if (bytes.size() >= 3 && (unsigned char)bytes[0] == 0xEF && (unsigned char)bytes[1] == 0xBB && (unsigned char)bytes[2] == 0xBF)
        bytes.erase(0, 3);
    return Wide(bytes);
}

void BeatDropLyricsManager::WriteUtf8File(const std::wstring& path, const std::wstring& text)
{
    std::ofstream file(path, std::ios::binary | std::ios::trunc);
    if (!file) return;
    const std::string bytes = Utf8(text);
    file.write(bytes.data(), (std::streamsize)bytes.size());
}

bool BeatDropLyricsManager::FetchLyrics(const std::wstring& artist, const std::wstring& title,
                                        const std::wstring& album, double durationSeconds,
                                        std::vector<BeatDropLyricLine>& lines, std::wstring& rawLrc)
{
#if defined(_WIN10_BUILD)
    if (FetchLyricsWinHttp(artist, title, album, durationSeconds, lines, rawLrc))
        return true;
    try
    {
        winrt::Windows::Web::Http::HttpClient client;
        client.DefaultRequestHeaders().Append(L"User-Agent", L"BeatDrop Music Visualizer");
        std::ostringstream query;
        query << "https://lrclib.net/api/get?track_name=" << UrlEncode(title)
              << "&artist_name=" << UrlEncode(artist)
              << "&album_name=" << UrlEncode(album)
              << "&duration=" << (int)((durationSeconds > 0.0) ? std::round(durationSeconds) : 0.0);
        auto response = client.GetAsync(winrt::Windows::Foundation::Uri(winrt::to_hstring(query.str()))).get();
        if (response.IsSuccessStatusCode())
        {
            const std::wstring body = response.Content().ReadAsStringAsync().get().c_str();
            const auto json = winrt::Windows::Data::Json::JsonObject::Parse(body);
            if (json.HasKey(L"instrumental") && json.GetNamedBoolean(L"instrumental", false))
                return true;
            const std::wstring synced = json.GetNamedString(L"syncedLyrics", L"").c_str();
            lines = ParseLrc(synced);
            if (!lines.empty())
            {
                rawLrc = synced;
                return true;
            }
        }

        std::ostringstream search;
        search << "https://lrclib.net/api/search?track_name=" << UrlEncode(title)
               << "&artist_name=" << UrlEncode(artist);
        response = client.GetAsync(winrt::Windows::Foundation::Uri(winrt::to_hstring(search.str()))).get();
        if (!response.IsSuccessStatusCode())
            return false;
        const auto results = winrt::Windows::Data::Json::JsonArray::Parse(
            response.Content().ReadAsStringAsync().get());
        for (const auto& value : results)
        {
            const auto item = value.GetObject();
            const std::wstring synced = item.GetNamedString(L"syncedLyrics", L"").c_str();
            lines = ParseLrc(synced);
            if (!lines.empty())
            {
                rawLrc = synced;
                return true;
            }
        }
        return false;
    }
    catch (...) { return false; }
#else
    (void)artist; (void)title; (void)album; (void)durationSeconds; (void)lines; (void)rawLrc;
    return false;
#endif
}
