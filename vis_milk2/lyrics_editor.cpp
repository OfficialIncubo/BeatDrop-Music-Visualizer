#include "lyrics_editor.h"

#include "lyrics_lrc.h"
#include "lyrics_manager.h"
#include "songtitlegetter.h"

#include <algorithm>
#include <array>
#include <bcrypt.h>
#include <cctype>
#include <cmath>
#include <commctrl.h>
#include <commdlg.h>
#include <cstdint>
#include <cwctype>
#include <fstream>
#include <memory>
#include <richedit.h>
#include <sstream>
#include <thread>
#include <winhttp.h>

extern SongTitleGetter songtitlegetter;

namespace
{
    constexpr int ID_INPUT = 1000;
    constexpr int ID_PARSED = 1001;
    constexpr int ID_PARSE = 1002;
    constexpr int ID_IMPORT = 1003;
    constexpr int ID_DELETE = 1004;
    constexpr int ID_INSERT = 1005;
    constexpr int ID_UNDO = 1006;
    constexpr int ID_REDO = 1007;
    constexpr int ID_SEEK = 1008;
    constexpr int ID_RESET = 1009;
    constexpr int ID_CAPTURE = 1010;
    constexpr int ID_MINUS5 = 1011;
    constexpr int ID_PLUS5 = 1012;
    constexpr int ID_UP = 1013;
    constexpr int ID_DOWN = 1014;
    constexpr int ID_SAVE = 1015;
    constexpr int ID_CLOSE = 1016;
    constexpr int ID_STATUS = 1017;
    constexpr int ID_INPUT_LABEL = 1018;
    constexpr int ID_PARSED_LABEL = 1019;
    constexpr int ID_MINUS30 = 1020;
    constexpr int ID_MINUS10 = 1021;
    constexpr int ID_PLUS10 = 1022;
    constexpr int ID_PLUS30 = 1023;
    constexpr int ID_UPLOAD = 1024;
    constexpr int ID_AUTO_APPLY = 1025;
    constexpr int ID_CLEAR_CACHE = 1026;
    constexpr UINT_PTR STATUS_TIMER = 1;
    constexpr UINT UPLOAD_FINISHED = WM_APP + 42;
    const wchar_t* kClassName = L"BeatDropLyricsEditorWindow";

    const wchar_t* RichEditClass()
    {
        // Rich Edit registers window procedures backed by this DLL.  Keep one
        // module reference for the application's lifetime rather than loading
        // once per editor window or unloading during Rich Edit's teardown.
        static HMODULE module = nullptr;
        static bool initialized = false;
        static bool usesMsftEdit = false;
        if (!initialized)
        {
            initialized = true;
            module = LoadLibraryW(L"Msftedit.dll");
            usesMsftEdit = module != nullptr;
            if (!module)
                module = LoadLibraryW(L"Riched20.dll");
        }
        return usesMsftEdit ? MSFTEDIT_CLASS : L"RichEdit20W";
    }

    HBRUSH WindowBrush()
    {
        static HBRUSH brush = CreateSolidBrush(RGB(30, 30, 34));
        return brush;
    }

    HBRUSH PaneBrush()
    {
        static HBRUSH brush = CreateSolidBrush(RGB(43, 43, 49));
        return brush;
    }

    HFONT EditorFont()
    {
        static HFONT font = CreateFontW(-15, 0, 0, 0, FW_NORMAL, FALSE, FALSE,
            FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
        return font;
    }

    HFONT StrongEditorFont()
    {
        static HFONT font = CreateFontW(-15, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE,
            FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
        return font;
    }

    void MakePill(HWND button, int width)
    {
        if (button)
            SetWindowRgn(button, CreateRoundRectRgn(0, 0, width, 32, 14, 14), TRUE);
    }

    void DrawHistoryIcon(HDC dc, const RECT& bounds, bool redo, bool enabled)
    {
        const int centreX = bounds.left + 16;
        const int centreY = (bounds.top + bounds.bottom) / 2;
        const COLORREF color = enabled ? RGB(244, 244, 248) : RGB(120, 120, 126);
        HPEN pen = CreatePen(PS_SOLID, 2, color);
        HBRUSH brush = CreateSolidBrush(color);
        HGDIOBJ oldPen = SelectObject(dc, pen);
        HGDIOBJ oldBrush = SelectObject(dc, brush);

        // Each head starts at the end of its arc and points along that arc's
        // tangent. Undo is the reverse of Redo.
        if (redo)
            Arc(dc, centreX - 8, centreY - 8, centreX + 8, centreY + 8,
                centreX + 5, centreY - 5, centreX - 6, centreY + 4);
        else
            Arc(dc, centreX - 8, centreY - 8, centreX + 8, centreY + 8,
                centreX - 5, centreY + 5, centreX + 6, centreY - 4);

        POINT arrow[3] = {};
        if (redo)
        {
            // Attach the same down-right head to the arc's other cap (+5, -5)
            // without rotating it.  The cap lies across this triangle's base.
            arrow[0] = {centreX + 10, centreY};
            arrow[1] = {centreX + 2, centreY - 2};
            arrow[2] = {centreX + 8, centreY - 8};
        }
        else
        {
            // The reverse arc ends at (+6, -4), then continues up-left.
            arrow[0] = {centreX + 1, centreY - 9};
            arrow[1] = {centreX + 9, centreY - 7};
            arrow[2] = {centreX + 3, centreY - 1};
        }
        Polygon(dc, arrow, 3);

        SelectObject(dc, oldBrush);
        SelectObject(dc, oldPen);
        DeleteObject(brush);
        DeleteObject(pen);
    }

    void DrawReorderArrow(HDC dc, const RECT& bounds, bool up, bool enabled)
    {
        const int centreX = (bounds.left + bounds.right) / 2;
        const int centreY = (bounds.top + bounds.bottom) / 2;
        const COLORREF color = enabled ? RGB(244, 244, 248) : RGB(120, 120, 126);
        HBRUSH brush = CreateSolidBrush(color);
        HGDIOBJ oldBrush = SelectObject(dc, brush);
        HGDIOBJ oldPen = SelectObject(dc, GetStockObject(NULL_PEN));

        POINT arrow[7] = {};
        if (up)
        {
            arrow[0] = {centreX, centreY - 9};
            arrow[1] = {centreX - 8, centreY - 1};
            arrow[2] = {centreX - 4, centreY - 1};
            arrow[3] = {centreX - 4, centreY + 8};
            arrow[4] = {centreX + 4, centreY + 8};
            arrow[5] = {centreX + 4, centreY - 1};
            arrow[6] = {centreX + 8, centreY - 1};
        }
        else
        {
            arrow[0] = {centreX, centreY + 9};
            arrow[1] = {centreX - 8, centreY + 1};
            arrow[2] = {centreX - 4, centreY + 1};
            arrow[3] = {centreX - 4, centreY - 8};
            arrow[4] = {centreX + 4, centreY - 8};
            arrow[5] = {centreX + 4, centreY + 1};
            arrow[6] = {centreX + 8, centreY + 1};
        }
        Polygon(dc, arrow, static_cast<int>(_countof(arrow)));

        SelectObject(dc, oldPen);
        SelectObject(dc, oldBrush);
        DeleteObject(brush);
    }

    std::string JsonEscape(const std::string& value)
    {
        std::string escaped;
        escaped.reserve(value.size() + 16);
        for (const unsigned char ch : value)
        {
            switch (ch)
            {
            case '\\': escaped += "\\\\"; break;
            case '"': escaped += "\\\""; break;
            case '\n': escaped += "\\n"; break;
            case '\r': escaped += "\\r"; break;
            case '\t': escaped += "\\t"; break;
            default:
                if (ch < 0x20)
                {
                    char encoded[7] = {};
                    sprintf_s(encoded, "\\u%04x", ch);
                    escaped += encoded;
                }
                else
                    escaped += static_cast<char>(ch);
                break;
            }
        }
        return escaped;
    }

    std::string JsonStringValue(const std::string& body, const char* key)
    {
        const std::string marker = std::string("\"") + key + "\"";
        size_t at = body.find(marker);
        if (at == std::string::npos)
            return std::string();
        at = body.find(':', at + marker.size());
        if (at == std::string::npos)
            return std::string();
        at = body.find('"', at + 1);
        if (at == std::string::npos)
            return std::string();

        std::string result;
        for (++at; at < body.size(); ++at)
        {
            if (body[at] == '"')
                break;
            if (body[at] == '\\' && at + 1 < body.size())
            {
                const char escaped = body[++at];
                result += escaped == 'n' ? '\n' : escaped == 'r' ? '\r' :
                    escaped == 't' ? '\t' : escaped;
            }
            else
                result += body[at];
        }
        return result;
    }

    bool Sha256(const std::string& value, std::array<BYTE, 32>& digest)
    {
        BCRYPT_ALG_HANDLE algorithm = nullptr;
        BCRYPT_HASH_HANDLE hash = nullptr;
        DWORD objectLength = 0;
        DWORD resultLength = 0;
        bool ok = BCryptOpenAlgorithmProvider(&algorithm, BCRYPT_SHA256_ALGORITHM,
            nullptr, 0) == 0;
        if (ok)
            ok = BCryptGetProperty(algorithm, BCRYPT_OBJECT_LENGTH,
                reinterpret_cast<PUCHAR>(&objectLength), sizeof(objectLength),
                &resultLength, 0) == 0;
        std::vector<BYTE> object(objectLength);
        if (ok)
            ok = BCryptCreateHash(algorithm, &hash, object.data(), objectLength,
                nullptr, 0, 0) == 0;
        if (ok)
            ok = BCryptHashData(hash, reinterpret_cast<PUCHAR>(
                const_cast<char*>(value.data())), static_cast<ULONG>(value.size()), 0) == 0;
        if (ok)
            ok = BCryptFinishHash(hash, digest.data(), static_cast<ULONG>(digest.size()), 0) == 0;
        if (hash)
            BCryptDestroyHash(hash);
        if (algorithm)
            BCryptCloseAlgorithmProvider(algorithm, 0);
        return ok;
    }

    bool HashMeetsTarget(const std::array<BYTE, 32>& hash, const std::string& target)
    {
        if (target.size() != 64)
            return false;
        constexpr char hex[] = "0123456789abcdef";
        for (size_t index = 0; index < hash.size(); ++index)
        {
            const char expectedHigh = static_cast<char>(std::tolower(
                static_cast<unsigned char>(target[index * 2])));
            const char expectedLow = static_cast<char>(std::tolower(
                static_cast<unsigned char>(target[index * 2 + 1])));
            const char actualHigh = hex[hash[index] >> 4];
            const char actualLow = hex[hash[index] & 15];
            if (actualHigh != expectedHigh)
                return actualHigh < expectedHigh;
            if (actualLow != expectedLow)
                return actualLow < expectedLow;
        }
        return true;
    }

    std::string SolveChallenge(const std::string& prefix, const std::string& target)
    {
        // LRCLIB supplies the target.  This runs on the worker thread so even
        // a slow proof-of-work does not freeze text editing or playback sync.
        for (uint64_t nonce = 0; nonce < 1000000000ULL; ++nonce)
        {
            std::array<BYTE, 32> hash = {};
            const std::string candidate = prefix + std::to_string(nonce);
            if (Sha256(candidate, hash) && HashMeetsTarget(hash, target))
                return std::to_string(nonce);
        }
        return std::string();
    }

    bool PostLrclib(const wchar_t* path, const std::string& body,
        const std::wstring& extraHeaders, DWORD& status, std::string& response)
    {
        // DEFAULT_PROXY works on the supported Vista-and-newer line.  HTTPS
        // protocol support remains governed by the user\'s installed Windows
        // updates and TLS configuration.
        HINTERNET session = WinHttpOpen(L"BeatDrop Music Visualizer/1.5 "
            L"(https://github.com/OfficialIncubo/BeatDrop-Music-Visualizer)",
            WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME,
            WINHTTP_NO_PROXY_BYPASS, 0);
        if (!session)
            return false;
        WinHttpSetTimeouts(session, 10000, 10000, 20000, 20000);
        HINTERNET connection = WinHttpConnect(session, L"lrclib.net",
            INTERNET_DEFAULT_HTTPS_PORT, 0);
        HINTERNET request = connection ? WinHttpOpenRequest(connection, L"POST", path,
            nullptr, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES,
            WINHTTP_FLAG_SECURE) : nullptr;
        const std::wstring headers = L"Content-Type: application/json\r\nAccept: application/json\r\n" + extraHeaders;
        const bool ok = request && WinHttpSendRequest(request, headers.c_str(),
            static_cast<DWORD>(headers.size()), const_cast<char*>(body.data()),
            static_cast<DWORD>(body.size()), static_cast<DWORD>(body.size()), 0) &&
            WinHttpReceiveResponse(request, nullptr);
        if (ok)
        {
            DWORD size = sizeof(status);
            WinHttpQueryHeaders(request, WINHTTP_QUERY_STATUS_CODE |
                WINHTTP_QUERY_FLAG_NUMBER, WINHTTP_HEADER_NAME_BY_INDEX, &status,
                &size, WINHTTP_NO_HEADER_INDEX);
            DWORD available = 0;
            while (WinHttpQueryDataAvailable(request, &available) && available)
            {
                std::string chunk(available, '\0');
                DWORD read = 0;
                if (!WinHttpReadData(request, &chunk[0], available, &read) || !read)
                    break;
                response.append(chunk.data(), read);
            }
        }
        if (request)
            WinHttpCloseHandle(request);
        if (connection)
            WinHttpCloseHandle(connection);
        WinHttpCloseHandle(session);
        return ok;
    }

    bool FirstTimestamp(const std::wstring& line, size_t& start, size_t& end,
        double& seconds);

    std::wstring PlainLyrics(const std::wstring& syncedLyrics)
    {
        std::wistringstream input(syncedLyrics);
        std::wostringstream output;
        std::wstring row;
        bool firstLine = true;
        while (std::getline(input, row))
        {
            if (!row.empty() && row.back() == L'\r')
                row.pop_back();
            size_t start = 0;
            size_t end = 0;
            double seconds = 0.0;
            while (FirstTimestamp(row, start, end, seconds) && start == 0)
            {
                row.erase(0, end + 1);
                if (!row.empty() && row.front() == L' ')
                    row.erase(0, 1);
            }
            if (!firstLine)
                output << L"\n";
            output << row;
            firstLine = false;
        }
        return output.str();
    }

    struct UploadResult
    {
        bool succeeded = false;
        std::wstring message;
    };

    void PostUploadResult(HWND editorWindow, bool succeeded, const std::wstring& message)
    {
        std::unique_ptr<UploadResult> result(new UploadResult{succeeded, message});
        if (IsWindow(editorWindow) && PostMessageW(editorWindow, UPLOAD_FINISHED,
            0, reinterpret_cast<LPARAM>(result.get())))
            result.release();
    }

    bool LineRange(const std::wstring& document, int line, size_t& start, size_t& end)
    {
        if (line < 0)
            return false;
        start = 0;
        for (int index = 0; index < line; ++index)
        {
            const size_t newline = document.find(L'\n', start);
            if (newline == std::wstring::npos)
                return false;
            start = newline + 1;
        }
        end = document.find_first_of(L"\r\n", start);
        if (end == std::wstring::npos)
            end = document.size();
        return start <= document.size();
    }

    bool FirstTimestamp(const std::wstring& line, size_t& start, size_t& end, double& seconds)
    {
        start = line.find(L'[');
        if (start == std::wstring::npos)
            return false;
        end = line.find(L']', start + 1);
        if (end == std::wstring::npos)
            return false;
        return BeatDropLrc::ParseTimestamp(line.substr(start + 1, end - start - 1), seconds);
    }

    std::wstring FormatTimestamp(double seconds)
    {
        const long long centiseconds = (std::max)(0LL,
            static_cast<long long>(seconds * 100.0 + 0.5));
        wchar_t value[32] = {};
        swprintf_s(value, L"[%02lld:%02lld.%02lld]", centiseconds / 6000,
            (centiseconds / 100) % 60, centiseconds % 100);
        return value;
    }

    std::wstring ReadUtf8OrAnsi(const wchar_t* path)
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
        if (bytes.empty())
            return std::wstring();
        int count = MultiByteToWideChar(CP_UTF8, 0, bytes.data(),
            static_cast<int>(bytes.size()), nullptr, 0);
        UINT encoding = CP_UTF8;
        if (count <= 0)
        {
            encoding = CP_ACP;
            count = MultiByteToWideChar(encoding, 0, bytes.data(),
                static_cast<int>(bytes.size()), nullptr, 0);
        }
        if (count <= 0)
            return std::wstring();
        std::wstring result(static_cast<size_t>(count), L'\0');
        MultiByteToWideChar(encoding, 0, bytes.data(), static_cast<int>(bytes.size()),
            &result[0], count);
        return result;
    }

    // LRCLIB responses in the wild occasionally contain successive timestamped
    // lines without line breaks.  The input pane should always remain readable,
    // while consecutive timestamps with one shared lyric stay on one source row.
    std::wstring NormalizeInputText(const std::wstring& source)
    {
        std::wstring result;
        result.reserve(source.size() + 32);
        bool lineHasTimedTag = false;
        bool textAfterTimedTag = false;
        bool otherTextOnLine = false;

        for (size_t index = 0; index < source.size(); )
        {
            const wchar_t ch = source[index];
            if (ch == L'\r' || ch == L'\n')
            {
                if (result.empty() || result.back() != L'\n')
                    result += L"\r\n";
                if (ch == L'\r' && index + 1 < source.size() && source[index + 1] == L'\n')
                    ++index;
                ++index;
                lineHasTimedTag = false;
                textAfterTimedTag = false;
                otherTextOnLine = false;
                continue;
            }

            if (ch == L'[')
            {
                const size_t close = source.find(L']', index + 1);
                double ignoredSeconds = 0.0;
                if (close != std::wstring::npos && BeatDropLrc::ParseTimestamp(
                    source.substr(index + 1, close - index - 1), ignoredSeconds))
                {
                    // Break when a prior timestamp already owns lyric text, or
                    // after metadata/non-timed content.  Do not split [t][t]
                    // because those two tags legitimately share one lyric.
                    if (!result.empty() && result.back() != L'\n' &&
                        ((lineHasTimedTag && textAfterTimedTag) || otherTextOnLine))
                    {
                        result += L"\r\n";
                        lineHasTimedTag = false;
                        textAfterTimedTag = false;
                        otherTextOnLine = false;
                    }
                    result.append(source, index, close - index + 1);
                    lineHasTimedTag = true;
                    index = close + 1;
                    continue;
                }
            }

            result.push_back(ch);
            if (!iswspace(static_cast<wint_t>(ch)))
            {
                if (lineHasTimedTag)
                    textAfterTimedTag = true;
                else
                    otherTextOnLine = true;
            }
            ++index;
        }

        // The loop intentionally coalesces physical line endings. Restore the
        // semantic blank separator used between a leading LRC metadata block
        // and its first synchronized lyric.
        bool sawMetadata = false;
        for (size_t lineStart = 0; lineStart < result.size(); )
        {
            const size_t lineEnd = result.find(L'\n', lineStart);
            const size_t contentEnd = lineEnd == std::wstring::npos ? result.size() :
                (lineEnd > lineStart && result[lineEnd - 1] == L'\r' ? lineEnd - 1 : lineEnd);
            const std::wstring line = result.substr(lineStart, contentEnd - lineStart);
            const size_t close = line.find(L']');
            double seconds = 0.0;
            const bool timed = !line.empty() && line.front() == L'[' &&
                close != std::wstring::npos &&
                BeatDropLrc::ParseTimestamp(line.substr(1, close - 1), seconds);
            if (timed && sawMetadata)
            {
                result.insert(lineStart, L"\r\n");
                break;
            }
            if (!line.empty() && line.front() == L'[' && close != std::wstring::npos && !timed)
                sawMetadata = true;
            else if (!line.empty())
                break;
            if (lineEnd == std::wstring::npos)
                break;
            lineStart = lineEnd + 1;
        }
        return result;
    }

    std::wstring BuildParsedText(const std::wstring& normalizedInput)
    {
        // Keep every lyric row.  A partially synchronized LRC is a normal
        // editing state: parsed timestamps stay in place while plain rows are
        // retained for Capture timestamp instead of being dropped or sorted.
        std::wistringstream input(normalizedInput);
        std::wostringstream output;
        std::wstring row;
        bool inHeader = true;
        bool wroteLyric = false;
        while (std::getline(input, row))
        {
            if (!row.empty() && row.back() == L'\r')
                row.pop_back();

            const size_t close = row.find(L']');
            double seconds = 0.0;
            const bool timed = !row.empty() && row.front() == L'[' &&
                close != std::wstring::npos &&
                BeatDropLrc::ParseTimestamp(row.substr(1, close - 1), seconds);
            const bool metadata = !row.empty() && row.front() == L'[' &&
                close != std::wstring::npos && !timed;
            if (inHeader && (metadata || row.empty()))
                continue;
            inHeader = false;

            if (wroteLyric)
                output << L"\r\n";
            output << row;
            wroteLyric = true;
        }
        return output.str();
    }

    // Metadata is intentionally edited in the Input pane.  Parsed Text only
    // contains timed lyrics, so extract the leading non-timestamp LRC tags
    // before saving and leave every user edit to that header intact.
    std::wstring EditableHeader(const std::wstring& input)
    {
        std::wistringstream lines(input);
        std::wstring row;
        std::wostringstream header;
        bool foundHeader = false;

        while (std::getline(lines, row))
        {
            if (!row.empty() && row.back() == L'\r')
                row.pop_back();
            if (row.empty())
            {
                if (foundHeader)
                    continue;
                continue;
            }

            const size_t close = row.find(L']');
            double seconds = 0.0;
            const bool isTimedLine = row.front() == L'[' && close != std::wstring::npos &&
                BeatDropLrc::ParseTimestamp(row.substr(1, close - 1), seconds);
            if (row.front() != L'[' || close == std::wstring::npos || isTimedLine)
                break;

            if (foundHeader)
                header << L"\r\n";
            header << row;
            foundHeader = true;
        }
        return header.str();
    }

    std::wstring DocumentForSave(const std::wstring& input,
        const std::wstring& synchronizedLyrics, bool& hasEditableHeader)
    {
        const std::wstring header = EditableHeader(input);
        hasEditableHeader = !header.empty();
        if (!hasEditableHeader)
            return synchronizedLyrics;

        size_t firstLyric = 0;
        while (firstLyric < synchronizedLyrics.size() &&
            (synchronizedLyrics[firstLyric] == L'\r' || synchronizedLyrics[firstLyric] == L'\n'))
            ++firstLyric;
        // Exactly one empty line separates LRC metadata from its timed script.
        return header + L"\r\n\r\n" + synchronizedLyrics.substr(firstLyric);
    }
}

void BeatDropLyricsEditor::Open(HWND owner, BeatDropLyricsManager* manager,
    const std::wstring& artist, const std::wstring& title)
{
    m_manager = manager;
    m_artist = artist;
    m_title = title;
    if (!m_hwnd)
    {
        WNDCLASSW windowClass = {};
        windowClass.lpfnWndProc = WindowProc;
        windowClass.hInstance = GetModuleHandleW(nullptr);
        windowClass.hCursor = LoadCursorW(nullptr, MAKEINTRESOURCEW(32512));
        windowClass.hbrBackground = WindowBrush();
        windowClass.lpszClassName = kClassName;
        RegisterClassW(&windowClass);
        CreateWindowExW(WS_EX_APPWINDOW, kClassName, L"BeatDrop Lyrics Editor",
            WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT,
            1160, 720, owner, nullptr, windowClass.hInstance, this);
        return;
    }

    const std::wstring source = NormalizeInputText(
        m_manager ? m_manager->CurrentLrc() : std::wstring());
    m_lastLoadedLrc = source;
    Write(m_input, source);
    Write(m_parsed, BuildParsedText(source));
    m_lastAppliedParsed = Read(m_parsed);
    m_undo.clear();
    m_redo.clear();
    FormatParsedText();
    UpdateHistoryButtons();
    SetWindowTextW(m_hwnd, (L"BeatDrop Lyrics Editor - " + m_artist + L" - " + m_title).c_str());
    ShowWindow(m_hwnd, SW_SHOWNORMAL);
    SetForegroundWindow(m_hwnd);
}

LRESULT CALLBACK BeatDropLyricsEditor::WindowProc(HWND hwnd, UINT message,
    WPARAM wParam, LPARAM lParam)
{
    BeatDropLyricsEditor* self = reinterpret_cast<BeatDropLyricsEditor*>(
        GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    if (message == WM_NCCREATE)
    {
        self = reinterpret_cast<BeatDropLyricsEditor*>(
            reinterpret_cast<CREATESTRUCTW*>(lParam)->lpCreateParams);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
        if (self)
            self->m_hwnd = hwnd;
    }
    return self ? self->HandleMessage(message, wParam, lParam) :
        DefWindowProcW(hwnd, message, wParam, lParam);
}

LRESULT CALLBACK BeatDropLyricsEditor::ParsedEditProc(HWND hwnd, UINT message,
    WPARAM wParam, LPARAM lParam)
{
    BeatDropLyricsEditor* self = reinterpret_cast<BeatDropLyricsEditor*>(
        GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    const WNDPROC oldProc = self && hwnd == self->m_input ?
        self->m_oldInputProc : (self ? self->m_oldParsedProc : nullptr);
    if (!self || !oldProc)
        return DefWindowProcW(hwnd, message, wParam, lParam);

    const bool control = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
    const bool alt = (GetKeyState(VK_MENU) & 0x8000) != 0;
    const bool parsedPane = hwnd == self->m_parsed;
    // WM_KEYDOWN starts timestamp capture, but Windows then emits WM_CHAR for
    // that same physical Space key.  Do not let it overwrite the next line
    // that capture just selected.
    if (parsedPane && message == WM_CHAR && wParam == L' ' && !control && !alt)
        return 0;
    if (message == WM_KEYDOWN)
    {
        if (control && (wParam == 'Z' || wParam == 'z'))
        {
            self->Undo();
            return 0;
        }
        if (control && (wParam == 'Y' || wParam == 'y'))
        {
            self->Redo();
            return 0;
        }
        if (parsedPane && wParam == VK_DELETE && !control)
        {
            self->ResetSelectedTimestamp();
            return 0;
        }
        if (parsedPane && wParam == VK_SPACE && !control && !alt)
        {
            self->CaptureAndAdvance();
            return 0;
        }
        if (!control && !alt && (wParam == VK_BACK || wParam == VK_RETURN ||
            (wParam >= 0x20 && wParam <= 0x7e)))
            self->PushUndo();
    }
    else if (message == WM_PASTE || message == WM_CUT || message == WM_CLEAR)
        self->PushUndo();

    const LRESULT result = CallWindowProcW(oldProc, hwnd,
        message, wParam, lParam);
    // Rich Edit does not reliably bubble EN_CHANGE on all supported Windows
    // versions.  Apply after the control accepts a user content edit instead.
    if (parsedPane && !self->m_writingText &&
        (message == WM_CHAR || message == WM_PASTE || message == WM_CUT ||
            message == WM_CLEAR || message == WM_UNDO))
        self->ApplyParsedToRenderer();
    if (message == WM_LBUTTONUP || message == WM_KEYUP)
        self->HandleEditorSelection();
    return result;
}

LRESULT BeatDropLyricsEditor::HandleMessage(UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
    {
        const wchar_t* richEditClass = RichEditClass();
        const auto menuId = [](int id) {
            return reinterpret_cast<HMENU>(static_cast<INT_PTR>(id));
        };
        const DWORD editStyle = WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE |
            ES_AUTOVSCROLL | ES_WANTRETURN;
        m_input = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"", editStyle,
            0, 0, 1, 1, m_hwnd, menuId(ID_INPUT), nullptr, nullptr);
        m_parsed = CreateWindowExW(WS_EX_CLIENTEDGE, richEditClass, L"", editStyle,
            0, 0, 1, 1, m_hwnd, menuId(ID_PARSED), nullptr, nullptr);
        SetWindowLongPtrW(m_parsed, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
        m_oldParsedProc = reinterpret_cast<WNDPROC>(SetWindowLongPtrW(m_parsed,
            GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(ParsedEditProc)));
        SetWindowLongPtrW(m_input, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
        m_oldInputProc = reinterpret_cast<WNDPROC>(SetWindowLongPtrW(m_input,
            GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(ParsedEditProc)));

        const DWORD buttonStyle = WS_CHILD | WS_VISIBLE | BS_OWNERDRAW;
        const auto button = [&](const wchar_t* caption, int id) {
            return CreateWindowW(L"BUTTON", caption, buttonStyle, 0, 0, 1, 1,
                m_hwnd, menuId(id), nullptr, nullptr);
        };
        button(L"Parse text", ID_PARSE);
        button(L"Import LRC", ID_IMPORT);
        button(L"Delete line", ID_DELETE);
        button(L"Insert silence", ID_INSERT);
        button(L"Reset editor", ID_RESET);
        button(L"Undo", ID_UNDO);
        button(L"Redo", ID_REDO);
        button(L"Seek to line", ID_SEEK);
        button(L"Upload to LRCLIB", ID_UPLOAD);
        button(L"Clear lyrics cache", ID_CLEAR_CACHE);
        button(L"Capture timestamp", ID_CAPTURE);
        button(L"-30s", ID_MINUS30);
        button(L"-10s", ID_MINUS10);
        button(L"-5s", ID_MINUS5);
        button(L"+5s", ID_PLUS5);
        button(L"+10s", ID_PLUS10);
        button(L"+30s", ID_PLUS30);
        button(L"", ID_UP);
        button(L"", ID_DOWN);
        button(L"Save local LRC", ID_SAVE);
        button(L"Close", ID_CLOSE);
        CreateWindowW(L"STATIC", L"Input", WS_CHILD | WS_VISIBLE, 0, 0, 1, 1,
            m_hwnd, menuId(ID_INPUT_LABEL), nullptr, nullptr);
        CreateWindowW(L"STATIC", L"Parsed Text",
            WS_CHILD | WS_VISIBLE, 0, 0, 1, 1, m_hwnd, menuId(ID_PARSED_LABEL), nullptr, nullptr);
        HWND autoApply = CreateWindowW(L"BUTTON", L"Auto-apply to renderer",
            WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX, 0, 0, 1, 1,
            m_hwnd, menuId(ID_AUTO_APPLY), nullptr, nullptr);
        SendMessageW(autoApply, BM_SETCHECK, BST_CHECKED, 0);
        m_status = CreateWindowW(L"STATIC", L"Lyrics ready",
            WS_CHILD | WS_VISIBLE | SS_CENTER | SS_ENDELLIPSIS,
            0, 0, 1, 1, m_hwnd, menuId(ID_STATUS), nullptr, nullptr);

        for (HWND control : {m_input, m_parsed, GetDlgItem(m_hwnd, ID_INPUT_LABEL),
            GetDlgItem(m_hwnd, ID_PARSED_LABEL), autoApply, m_status})
            SendMessageW(control, WM_SETFONT, reinterpret_cast<WPARAM>(EditorFont()), TRUE);
        SendMessageW(m_parsed, EM_SETBKGNDCOLOR, 0, RGB(43, 43, 49));
        const std::wstring source = NormalizeInputText(
            m_manager ? m_manager->CurrentLrc() : std::wstring());
        m_lastLoadedLrc = source;
        Write(m_input, source);
        Write(m_parsed, BuildParsedText(source));
        m_lastAppliedParsed = Read(m_parsed);
        FormatParsedText();
        UpdateHistoryButtons();
        SetWindowTextW(m_hwnd, (L"BeatDrop Lyrics Editor - " + m_artist + L" - " + m_title).c_str());
        SetTimer(m_hwnd, STATUS_TIMER, 500, nullptr);
        ResizeControls();
        return 0;
    }
    case WM_SIZE:
        ResizeControls();
        return 0;
    case WM_GETMINMAXINFO:
    {
        MINMAXINFO* info = reinterpret_cast<MINMAXINFO*>(lParam);
        info->ptMinTrackSize.x = 1020;
        info->ptMinTrackSize.y = 580;
        return 0;
    }
    case WM_TIMER:
        if (m_manager)
        {
            const std::wstring currentArtist = songtitlegetter.currentArtist;
            const std::wstring currentTitle = songtitlegetter.currentTitle;
            if (currentArtist != m_artist || currentTitle != m_title)
            {
                // The editor owns a document for exactly one track.  Switch
                // identity immediately so stale lyrics can never be saved to
                // the newly playing artist/title while the lookup is pending.
                m_artist = currentArtist;
                m_title = currentTitle;
                m_lastLoadedLrc.clear();
                Write(m_input, std::wstring());
                Write(m_parsed, std::wstring());
                m_lastAppliedParsed.clear();
                m_undo.clear();
                m_redo.clear();
                FormatParsedText();
                UpdateHistoryButtons();

                const std::wstring displayName = m_artist.empty() && m_title.empty() ?
                    L"No active track" : (m_artist.empty() ? m_title :
                        (m_title.empty() ? m_artist : m_artist + L" - " + m_title));
                SetWindowTextW(m_hwnd, (L"BeatDrop Lyrics Editor - " + displayName).c_str());
                if (!m_title.empty())
                {
                    m_manager->UpdateTrack(m_artist, m_title, songtitlegetter.currentAlbum,
                        songtitlegetter.GetDurationMilliseconds() / 1000.0);
                    m_manager->Refresh();
                    SetStatus(L"Looking up lyrics for the new track...");
                }
                else
                {
                    m_manager->UpdateTrack(std::wstring(), std::wstring(), std::wstring(), 0.0);
                    SetStatus(L"Waiting for a now-playing track.");
                }
            }

            const std::wstring fetched = NormalizeInputText(m_manager->CurrentLrc());
            if (!fetched.empty() && fetched != m_lastLoadedLrc &&
                Read(m_input) == m_lastLoadedLrc &&
                Read(m_parsed) == BuildParsedText(m_lastLoadedLrc))
            {
                m_lastLoadedLrc = fetched;
                Write(m_input, fetched);
                Write(m_parsed, BuildParsedText(fetched));
                m_lastAppliedParsed = Read(m_parsed);
                FormatParsedText();
                SetStatus(BeatDropLrc::Parse(fetched).empty() ?
                    L"Loaded unsynchronized lyrics; capture timestamps to synchronize them." :
                    L"Loaded synchronized lyrics for the current track.");
            }
            else if (fetched.empty() && m_manager->Status() == L"No lyrics found")
            {
                SetStatus(L"No lyrics found for this track. Import or paste lyrics to synchronize them.");
            }

            // Covers IME/input-method edits and Rich Edit notification gaps
            // that exist on older Windows controls.
            if (AutoApplyEnabled() && Read(m_parsed) != m_lastAppliedParsed)
                ApplyParsedToRenderer();
        }
        return 0;
    case WM_ERASEBKGND:
    {
        RECT bounds = {};
        GetClientRect(m_hwnd, &bounds);
        FillRect(reinterpret_cast<HDC>(wParam), &bounds, WindowBrush());
        return 1;
    }
    case WM_CTLCOLORSTATIC:
    case WM_CTLCOLORBTN:
    {
        HDC dc = reinterpret_cast<HDC>(wParam);
        SetTextColor(dc, RGB(232, 232, 238));
        SetBkColor(dc, RGB(30, 30, 34));
        return reinterpret_cast<LRESULT>(WindowBrush());
    }
    case WM_CTLCOLOREDIT:
    {
        HDC dc = reinterpret_cast<HDC>(wParam);
        SetTextColor(dc, RGB(232, 232, 238));
        SetBkColor(dc, RGB(43, 43, 49));
        return reinterpret_cast<LRESULT>(PaneBrush());
    }
    case WM_DRAWITEM:
    {
        DRAWITEMSTRUCT* item = reinterpret_cast<DRAWITEMSTRUCT*>(lParam);
        if (!item)
            return 0;
        const bool primary = item->CtlID == ID_SAVE;
        const bool pressed = (item->itemState & ODS_SELECTED) != 0;
        const bool disabled = (item->itemState & ODS_DISABLED) != 0;
        const COLORREF fillColor = disabled ? RGB(48, 48, 53) : primary ?
            (pressed ? RGB(105, 44, 196) : RGB(133, 57, 226)) :
            (pressed ? RGB(72, 72, 80) : RGB(57, 57, 64));
        HBRUSH fill = CreateSolidBrush(fillColor);
        FillRect(item->hDC, &item->rcItem, WindowBrush());
        HGDIOBJ oldBrush = SelectObject(item->hDC, fill);
        RoundRect(item->hDC, item->rcItem.left, item->rcItem.top,
            item->rcItem.right, item->rcItem.bottom, 14, 14);
        SelectObject(item->hDC, oldBrush);
        DeleteObject(fill);
        SetBkMode(item->hDC, TRANSPARENT);
        SetTextColor(item->hDC, disabled ? RGB(125, 125, 132) : RGB(246, 246, 250));
        wchar_t caption[80] = {};
        GetWindowTextW(item->hwndItem, caption, static_cast<int>(_countof(caption)));
        if (item->CtlID == ID_UNDO || item->CtlID == ID_REDO)
        {
            DrawHistoryIcon(item->hDC, item->rcItem, item->CtlID == ID_REDO, !disabled);
            RECT label = item->rcItem;
            label.left += 32;
            SelectObject(item->hDC, StrongEditorFont());
            DrawTextW(item->hDC, caption, -1, &label, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
        }
        else if (item->CtlID == ID_UP || item->CtlID == ID_DOWN)
        {
            DrawReorderArrow(item->hDC, item->rcItem, item->CtlID == ID_UP, !disabled);
        }
        else
        {
            SelectObject(item->hDC, StrongEditorFont());
            DrawTextW(item->hDC, caption, -1, &item->rcItem,
                DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        }
        return 1;
    }
    case WM_COMMAND:
        if (LOWORD(wParam) == ID_PARSED && HIWORD(wParam) == EN_CHANGE &&
            !m_writingText && GetFocus() == m_parsed)
        {
            ApplyParsedToRenderer();
            return 0;
        }
        if (LOWORD(wParam) == ID_AUTO_APPLY && HIWORD(wParam) == BN_CLICKED)
        {
            if (AutoApplyEnabled())
            {
                ApplyParsedToRenderer(true);
                SetStatus(L"Auto-apply to renderer is enabled.");
            }
            else
                SetStatus(L"Auto-apply to renderer is disabled.");
            return 0;
        }
        switch (LOWORD(wParam))
        {
        case ID_PARSE: ParseText(); break;
        case ID_IMPORT: Import(); break;
        case ID_DELETE: DeleteSelectedLine(); break;
        case ID_INSERT: InsertSilence(); break;
        case ID_UNDO: Undo(); break;
        case ID_REDO: Redo(); break;
        case ID_SEEK: SeekToLine(); break;
        case ID_RESET: Reset(); break;
        case ID_UPLOAD: Upload(); break;
        case ID_CLEAR_CACHE: ClearCache(); break;
        case ID_CAPTURE: CaptureTimestamp(); break;
        case ID_MINUS30: songtitlegetter.SeekRelative(-30.0); break;
        case ID_MINUS10: songtitlegetter.SeekRelative(-10.0); break;
        case ID_MINUS5: songtitlegetter.SeekRelative(-5.0); break;
        case ID_PLUS5: songtitlegetter.SeekRelative(5.0); break;
        case ID_PLUS10: songtitlegetter.SeekRelative(10.0); break;
        case ID_PLUS30: songtitlegetter.SeekRelative(30.0); break;
        case ID_UP: MoveSelectedLine(-1); break;
        case ID_DOWN: MoveSelectedLine(1); break;
        case ID_SAVE: Save(); break;
        case ID_CLOSE: DestroyWindow(m_hwnd); break;
        }
        return 0;
    case WM_CLOSE:
        DestroyWindow(m_hwnd);
        return 0;
    case UPLOAD_FINISHED:
    {
        std::unique_ptr<UploadResult> result(
            reinterpret_cast<UploadResult*>(lParam));
        if (!result)
            return 0;
        SetStatus(result->message);
        MessageBoxW(m_hwnd, result->message.c_str(), L"LRCLIB upload",
            MB_OK | (result->succeeded ? MB_ICONINFORMATION : MB_ICONERROR));
        return 0;
    }
    case WM_DESTROY:
        KillTimer(m_hwnd, STATUS_TIMER);
        if (m_parsed && m_oldParsedProc)
            SetWindowLongPtrW(m_parsed, GWLP_WNDPROC,
                reinterpret_cast<LONG_PTR>(m_oldParsedProc));
        m_oldParsedProc = nullptr;
        m_hwnd = m_input = m_parsed = m_status = nullptr;
        return 0;
    }
    return DefWindowProcW(m_hwnd, message, wParam, lParam);
}

void BeatDropLyricsEditor::ResizeControls()
{
    if (!m_hwnd)
        return;
    RECT client = {};
    GetClientRect(m_hwnd, &client);
    const int width = client.right;
    const int height = client.bottom;
    // Keep the roomy, stable two-pane layout from the original editor.  The
    // controls have dedicated rows, so labels and buttons never overlap when
    // the user changes DPI or resizes the window.
    const int paneY = 74;
    const int bottomY = (std::max)(paneY + 110, height - 44);
    const int half = (std::max)(260, (width - 24) / 2);
    const int parsedX = 16 + half;
    const int parsedWidth = (std::max)(0, width - half - 52);
    const int autoApplyWidth = 190;
    const int parsedLabelWidth = (std::max)(80, parsedWidth - autoApplyWidth);
    MoveWindow(GetDlgItem(m_hwnd, ID_INPUT_LABEL), 8, 52, half - 8, 18, TRUE);
    MoveWindow(GetDlgItem(m_hwnd, ID_PARSED_LABEL), parsedX, 52, parsedLabelWidth, 18, TRUE);
    MoveWindow(GetDlgItem(m_hwnd, ID_AUTO_APPLY), parsedX + parsedLabelWidth, 49,
        autoApplyWidth, 22, TRUE);
    MoveWindow(m_input, 8, paneY, half - 4, bottomY - paneY - 4, TRUE);
    MoveWindow(m_parsed, parsedX, paneY, parsedWidth, bottomY - paneY - 4, TRUE);

    const int arrowX = width - 36;
    MoveWindow(GetDlgItem(m_hwnd, ID_UP), arrowX, paneY + 8, 28, 32, TRUE);
    MoveWindow(GetDlgItem(m_hwnd, ID_DOWN), arrowX, paneY + 48, 28, 32, TRUE);
    MakePill(GetDlgItem(m_hwnd, ID_UP), 28);
    MakePill(GetDlgItem(m_hwnd, ID_DOWN), 28);

    int x = 8;
    const auto topButton = [&](int id, int buttonWidth) {
        MoveWindow(GetDlgItem(m_hwnd, id), x, 8, buttonWidth, 32, TRUE);
        MakePill(GetDlgItem(m_hwnd, id), buttonWidth);
        x += buttonWidth + 5;
    };
    topButton(ID_PARSE, 100);
    topButton(ID_IMPORT, 100);
    topButton(ID_DELETE, 100);
    topButton(ID_INSERT, 110);
    topButton(ID_RESET, 105);
    topButton(ID_UNDO, 90);
    topButton(ID_REDO, 90);
    topButton(ID_SEEK, 110);
    topButton(ID_UPLOAD, 140);

    x = 8;
    const auto bottomButton = [&](int id, int buttonWidth) {
        MoveWindow(GetDlgItem(m_hwnd, id), x, bottomY + 4, buttonWidth, 32, TRUE);
        MakePill(GetDlgItem(m_hwnd, id), buttonWidth);
        x += buttonWidth + 4;
    };
    bottomButton(ID_CAPTURE, 150);
    bottomButton(ID_MINUS30, 48);
    bottomButton(ID_MINUS10, 48);
    bottomButton(ID_MINUS5, 48);
    bottomButton(ID_PLUS5, 48);
    bottomButton(ID_PLUS10, 52);
    bottomButton(ID_PLUS30, 52);
    bottomButton(ID_CLEAR_CACHE, 140);
    const int closeWidth = 84;
    const int saveWidth = 130;
    const int closeX = width - 8 - closeWidth;
    const int saveX = closeX - 8 - saveWidth;
    const int statusX = x + 8;
    const int statusWidth = (std::max)(0, saveX - statusX - 8);
    MoveWindow(m_status, statusX, bottomY + 8, statusWidth, 24, TRUE);
    MoveWindow(GetDlgItem(m_hwnd, ID_SAVE), saveX, bottomY + 4, 130, 32, TRUE);
    MakePill(GetDlgItem(m_hwnd, ID_SAVE), saveWidth);
    MoveWindow(GetDlgItem(m_hwnd, ID_CLOSE), closeX, bottomY + 4, closeWidth, 32, TRUE);
    MakePill(GetDlgItem(m_hwnd, ID_CLOSE), closeWidth);
    // Owner-drawn controls move while retaining a rounded window region.
    // Redrawing their old and new bounds prevents stale rounded fragments
    // from remaining after a live resize.
    RedrawWindow(m_hwnd, nullptr, nullptr, RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN);
}

std::wstring BeatDropLyricsEditor::Read(HWND control) const
{
    if (!control)
        return std::wstring();
    const int length = GetWindowTextLengthW(control);
    std::wstring value(static_cast<size_t>(length) + 1, L'\0');
    GetWindowTextW(control, &value[0], length + 1);
    value.resize(static_cast<size_t>(length));
    return value;
}

void BeatDropLyricsEditor::Write(HWND control, const std::wstring& text)
{
    if (control)
    {
        const bool wasWritingText = m_writingText;
        m_writingText = true;
        SetWindowTextW(control, text.c_str());
        m_writingText = wasWritingText;
    }
}

void BeatDropLyricsEditor::SetStatus(const std::wstring& text)
{
    if (m_status)
        SetWindowTextW(m_status, text.c_str());
}

bool BeatDropLyricsEditor::AutoApplyEnabled() const
{
    return m_hwnd && IsDlgButtonChecked(m_hwnd, ID_AUTO_APPLY) == BST_CHECKED;
}

void BeatDropLyricsEditor::ApplyParsedToRenderer(bool force)
{
    if (!m_manager || !m_parsed || !AutoApplyEnabled())
        return;
    const std::wstring text = Read(m_parsed);
    if (force || text != m_lastAppliedParsed)
    {
        m_manager->SetCurrentLrc(text);
        m_lastAppliedParsed = text;
    }
}

void BeatDropLyricsEditor::PushUndo()
{
    if (!m_input || !m_parsed || m_applyingHistory)
        return;
    const HistoryState snapshot = {Read(m_input), Read(m_parsed)};
    if (!m_undo.empty() && m_undo.back().input == snapshot.input &&
        m_undo.back().parsed == snapshot.parsed)
        return;
    m_undo.push_back(snapshot);
    if (m_undo.size() > 100)
        m_undo.erase(m_undo.begin());
    m_redo.clear();
    UpdateHistoryButtons();
}

void BeatDropLyricsEditor::UpdateHistoryButtons()
{
    if (!m_hwnd)
        return;
    EnableWindow(GetDlgItem(m_hwnd, ID_UNDO), !m_undo.empty());
    EnableWindow(GetDlgItem(m_hwnd, ID_REDO), !m_redo.empty());
    InvalidateRect(GetDlgItem(m_hwnd, ID_UNDO), nullptr, TRUE);
    InvalidateRect(GetDlgItem(m_hwnd, ID_REDO), nullptr, TRUE);
}

void BeatDropLyricsEditor::ParseText()
{
    const std::wstring source = NormalizeInputText(Read(m_input));
    PushUndo();
    Write(m_input, source);
    Write(m_parsed, BuildParsedText(source));
    FormatParsedText();
    ApplyParsedToRenderer();
    SetStatus(BeatDropLrc::Parse(source).empty() ?
        L"Plain lyric lines are ready; select each line and capture a timestamp." :
        L"Timed lyrics parsed into the editable pane.");
}

void BeatDropLyricsEditor::Save()
{
    if (!m_manager)
        return;
    const std::wstring lrc = Read(m_parsed);
    bool hasEditableHeader = false;
    const std::wstring document = DocumentForSave(Read(m_input), lrc, hasEditableHeader);
    const std::wstring path = m_manager->SaveCurrentLrc(document, hasEditableHeader);
    m_lastLoadedLrc = document;
    m_lastAppliedParsed = lrc;
    if (path.empty())
        SetStatus(L"Saved in memory; start a track before saving a local LRC file.");
    else if (BeatDropLrc::Parse(lrc).empty())
        SetStatus(L"Saved, but no valid timestamps were found.");
    else
        SetStatus(L"Saved local LRC: " + path);
}

void BeatDropLyricsEditor::ClearCache()
{
    if (!m_manager)
        return;

    const int confirmation = MessageBoxW(m_hwnd,
        L"Delete every local .lrc file in BeatDrop's lyrics cache?\n\n"
        L"This cannot be undone. The lyrics currently loaded in memory will remain available until the track changes.",
        L"Clear lyrics cache", MB_YESNO | MB_ICONWARNING | MB_DEFBUTTON2);
    if (confirmation != IDYES)
    {
        SetStatus(L"Keeping the local lyrics cache.");
        return;
    }

    const int deleted = m_manager->ClearLocalCache();
    if (deleted == 0)
        SetStatus(L"No local LRC files were found in the lyrics cache.");
    else
        SetStatus(L"Cleared " + std::to_wstring(deleted) +
            (deleted == 1 ? L" local LRC file." : L" local LRC files."));
}

void BeatDropLyricsEditor::Upload()
{
    if (!m_parsed)
        return;

    const std::wstring syncedLyrics = Read(m_parsed);
    if (BeatDropLrc::Parse(syncedLyrics).empty())
    {
        SetStatus(L"Add at least one timestamp before publishing to LRCLIB.");
        return;
    }
    if (m_artist.empty() || m_title.empty())
    {
        SetStatus(L"A current artist and title are required for an LRCLIB upload.");
        return;
    }

    const int confirmation = MessageBoxW(m_hwnd,
        L"This will publish the artist, title, album, duration, and lyric text "
        L"to the public LRCLIB service. LRCLIB also requires a proof-of-work "
        L"calculation before publishing.\n\nContinue?",
        L"Publish lyrics to LRCLIB", MB_YESNO | MB_ICONINFORMATION | MB_DEFBUTTON2);
    if (confirmation != IDYES)
        return;

    const HWND editorWindow = m_hwnd;
    const std::wstring artist = m_artist;
    const std::wstring title = m_title;
    const std::wstring album = songtitlegetter.currentAlbum;
    const int64_t durationMilliseconds = songtitlegetter.GetDurationMilliseconds();
    const double durationSeconds = durationMilliseconds > 0 ?
        durationMilliseconds / 1000.0 : 0.0;
    SetStatus(L"Requesting an LRCLIB publishing challenge...");

    std::thread([editorWindow, artist, title, album, syncedLyrics, durationSeconds]
    {
        const std::string syncedUtf8 = BeatDropLyricsManager::Utf8(syncedLyrics);
        const std::string payload =
            "{\"trackName\":\"" + JsonEscape(BeatDropLyricsManager::Utf8(title)) +
            "\",\"artistName\":\"" + JsonEscape(BeatDropLyricsManager::Utf8(artist)) +
            "\",\"albumName\":\"" + JsonEscape(BeatDropLyricsManager::Utf8(album)) +
            "\",\"duration\":" + std::to_string(durationSeconds) +
            ",\"plainLyrics\":\"" + JsonEscape(BeatDropLyricsManager::Utf8(
                PlainLyrics(syncedLyrics))) +
            "\",\"syncedLyrics\":\"" + JsonEscape(syncedUtf8) + "\"}";

        DWORD status = 0;
        std::string challenge;
        if (!PostLrclib(L"/api/request-challenge", "{}", L"", status, challenge))
        {
            PostUploadResult(editorWindow, false,
                L"Could not request an LRCLIB publishing challenge.");
            return;
        }
        const std::string prefix = JsonStringValue(challenge, "prefix");
        const std::string target = JsonStringValue(challenge, "target");
        if (prefix.empty() || target.empty())
        {
            PostUploadResult(editorWindow, false,
                L"LRCLIB returned an invalid publishing challenge.");
            return;
        }
        const std::string nonce = SolveChallenge(prefix, target);
        if (nonce.empty())
        {
            PostUploadResult(editorWindow, false,
                L"Could not solve the LRCLIB proof-of-work challenge.");
            return;
        }

        const std::wstring token = L"X-Publish-Token: " +
            BeatDropLyricsManager::Wide(prefix) + L":" +
            BeatDropLyricsManager::Wide(nonce) + L"\r\n";
        // This is not a batch, but leaving a short gap between the challenge
        // and publish calls follows LRCLIB's client-throttling guidance.
        Sleep(250);
        std::string response;
        if (!PostLrclib(L"/api/publish", payload, token, status, response) ||
            status < 200 || status >= 300)
        {
            PostUploadResult(editorWindow, false,
                L"LRCLIB rejected the upload (HTTP " + std::to_wstring(status) + L").");
            return;
        }
        PostUploadResult(editorWindow, true, L"Lyrics uploaded to LRCLIB.");
    }).detach();
}

void BeatDropLyricsEditor::Import()
{
    wchar_t path[32768] = {};
    OPENFILENAMEW dialog = {};
    dialog.lStructSize = sizeof(dialog);
    dialog.hwndOwner = m_hwnd;
    dialog.lpstrFilter = L"LRC lyrics\0*.lrc\0Text files\0*.txt\0All files\0*.*\0";
    dialog.lpstrFile = path;
    dialog.nMaxFile = static_cast<DWORD>(_countof(path));
    dialog.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
    if (!GetOpenFileNameW(&dialog))
        return;
    const std::wstring text = NormalizeInputText(ReadUtf8OrAnsi(path));
    PushUndo();
    Write(m_input, text);
    Write(m_parsed, BuildParsedText(text));
    if (m_manager)
        m_manager->SetCurrentLrc(text);
    m_lastLoadedLrc = text;
    m_lastAppliedParsed = Read(m_parsed);
    FormatParsedText();
    SetStatus(BeatDropLrc::Parse(text).empty() ?
        L"Imported lyrics into the editor; add timestamps to render them." :
        L"Imported synchronized lyrics are now active in the renderer.");
}

int BeatDropLyricsEditor::SelectedLine() const
{
    if (!m_parsed)
        return -1;
    CHARRANGE selection = {};
    SendMessageW(m_parsed, EM_EXGETSEL, 0, reinterpret_cast<LPARAM>(&selection));
    const LRESULT line = SendMessageW(m_parsed, EM_LINEFROMCHAR, selection.cpMin, 0);
    return line >= 0 ? static_cast<int>(line) : -1;
}

void BeatDropLyricsEditor::SelectLine(int line)
{
    if (!m_parsed || line < 0)
        return;
    const LRESULT start = SendMessageW(m_parsed, EM_LINEINDEX, line, 0);
    if (start < 0)
        return;
    const LRESULT length = SendMessageW(m_parsed, EM_LINELENGTH, start, 0);
    const LONG safeLength = length > 0 ? static_cast<LONG>(length) : 0;
    CHARRANGE selection = {static_cast<LONG>(start), static_cast<LONG>(start) + safeLength};
    SendMessageW(m_parsed, EM_EXSETSEL, 0, reinterpret_cast<LPARAM>(&selection));
    m_selectedLine = line;
}

void BeatDropLyricsEditor::CaptureTimestamp()
{
    const int line = SelectedLine();
    std::wstring document = Read(m_parsed);
    size_t start = 0;
    size_t end = 0;
    if (!LineRange(document, line, start, end))
        return;
    PushUndo();
    std::wstring row = document.substr(start, end - start);
    size_t timestampStart = 0;
    size_t timestampEnd = 0;
    double ignored = 0.0;
    const std::wstring timestamp = FormatTimestamp(songtitlegetter.GetPositionMilliseconds() / 1000.0);
    if (FirstTimestamp(row, timestampStart, timestampEnd, ignored))
        row.replace(timestampStart, timestampEnd - timestampStart + 1, timestamp);
    else
        row.insert(0, timestamp + L" ");
    document.replace(start, end - start, row);
    Write(m_parsed, document);
    FormatParsedText();
    ApplyParsedToRenderer();
    SelectLine(line + 1);
    SetFocus(m_parsed);
    SetStatus(L"Captured the current playback timestamp and selected the next line.");
}

void BeatDropLyricsEditor::CaptureAndAdvance()
{
    CaptureTimestamp();
}

void BeatDropLyricsEditor::ResetSelectedTimestamp()
{
    const int line = SelectedLine();
    std::wstring document = Read(m_parsed);
    size_t start = 0;
    size_t end = 0;
    if (!LineRange(document, line, start, end))
        return;
    std::wstring row = document.substr(start, end - start);
    size_t timestampStart = 0;
    size_t timestampEnd = 0;
    double ignored = 0.0;
    if (!FirstTimestamp(row, timestampStart, timestampEnd, ignored))
    {
        SetStatus(L"The selected line has no timestamp to remove.");
        return;
    }
    PushUndo();
    size_t removeEnd = timestampEnd + 1;
    if (removeEnd < row.size() && row[removeEnd] == L' ')
        ++removeEnd;
    row.erase(timestampStart, removeEnd - timestampStart);
    document.replace(start, end - start, row);
    Write(m_parsed, document);
    FormatParsedText();
    ApplyParsedToRenderer();
    SelectLine(line);
    SetStatus(L"Removed the selected timestamp.");
}

void BeatDropLyricsEditor::DeleteSelectedLine()
{
    const int line = SelectedLine();
    std::wstring document = Read(m_parsed);
    size_t start = 0;
    size_t end = 0;
    if (!LineRange(document, line, start, end))
        return;
    PushUndo();
    if (end < document.size() && document[end] == L'\r') ++end;
    if (end < document.size() && document[end] == L'\n') ++end;
    document.erase(start, end - start);
    Write(m_parsed, document);
    FormatParsedText();
    ApplyParsedToRenderer();
    SelectLine(line);
    SetStatus(L"Deleted the selected lyric line.");
}

void BeatDropLyricsEditor::InsertSilence()
{
    const int line = SelectedLine();
    std::wstring document = Read(m_parsed);
    size_t start = 0;
    size_t end = 0;
    if (!LineRange(document, line, start, end))
        return;
    const std::wstring row = document.substr(start, end - start);
    size_t timestampStart = 0;
    size_t timestampEnd = 0;
    double seconds = 0.0;
    if (!FirstTimestamp(row, timestampStart, timestampEnd, seconds))
    {
        SetStatus(L"Capture or select a timestamped line before inserting silence.");
        return;
    }
    PushUndo();
    document.insert(end, L"\r\n" + FormatTimestamp(seconds + 5.0));
    Write(m_parsed, document);
    FormatParsedText();
    ApplyParsedToRenderer();
    SelectLine(line + 1);
    SetStatus(L"Inserted a five-second silence marker.");
}

void BeatDropLyricsEditor::MoveSelectedLine(int direction)
{
    const int line = SelectedLine();
    if (!direction || line < 0)
        return;
    std::wstring document = Read(m_parsed);
    std::vector<std::wstring> rows;
    std::wistringstream stream(document);
    std::wstring row;
    while (std::getline(stream, row))
    {
        if (!row.empty() && row.back() == L'\r')
            row.pop_back();
        rows.push_back(row);
    }
    const int target = line + direction;
    if (target < 0 || target >= static_cast<int>(rows.size()))
        return;
    PushUndo();
    std::swap(rows[static_cast<size_t>(line)], rows[static_cast<size_t>(target)]);
    std::wostringstream result;
    for (size_t i = 0; i < rows.size(); ++i)
    {
        if (i) result << L"\r\n";
        result << rows[i];
    }
    Write(m_parsed, result.str());
    FormatParsedText();
    ApplyParsedToRenderer();
    SelectLine(target);
}

void BeatDropLyricsEditor::SeekToLine()
{
    const int line = SelectedLine();
    std::wstring document = Read(m_parsed);
    size_t start = 0;
    size_t end = 0;
    if (!LineRange(document, line, start, end))
        return;
    const std::wstring row = document.substr(start, end - start);
    size_t timestampStart = 0;
    size_t timestampEnd = 0;
    double seconds = 0.0;
    if (!FirstTimestamp(row, timestampStart, timestampEnd, seconds))
    {
        SetStatus(L"Select a valid timestamped lyric line first.");
        return;
    }
    SetStatus(songtitlegetter.SeekTo(seconds) ? L"Seeked to the selected lyric line." :
        L"The active player does not expose a seekable now-playing session.");
}

void BeatDropLyricsEditor::Undo()
{
    if (!m_input || !m_parsed || m_undo.empty())
        return;
    const HWND focus = GetFocus();
    m_applyingHistory = true;
    m_redo.push_back({Read(m_input), Read(m_parsed)});
    Write(m_input, m_undo.back().input);
    Write(m_parsed, m_undo.back().parsed);
    m_undo.pop_back();
    m_applyingHistory = false;
    FormatParsedText();
    ApplyParsedToRenderer();
    UpdateHistoryButtons();
    SetFocus(focus == m_input ? m_input : m_parsed);
}

void BeatDropLyricsEditor::Redo()
{
    if (!m_input || !m_parsed || m_redo.empty())
        return;
    const HWND focus = GetFocus();
    m_applyingHistory = true;
    m_undo.push_back({Read(m_input), Read(m_parsed)});
    Write(m_input, m_redo.back().input);
    Write(m_parsed, m_redo.back().parsed);
    m_redo.pop_back();
    m_applyingHistory = false;
    FormatParsedText();
    ApplyParsedToRenderer();
    UpdateHistoryButtons();
    SetFocus(focus == m_input ? m_input : m_parsed);
}

void BeatDropLyricsEditor::Reset()
{
    const std::wstring source = NormalizeInputText(
        m_manager ? m_manager->CurrentLrc() : std::wstring());
    Write(m_input, source);
    Write(m_parsed, BuildParsedText(source));
    m_lastLoadedLrc = source;
    m_lastAppliedParsed = Read(m_parsed);
    m_undo.clear();
    m_redo.clear();
    FormatParsedText();
    UpdateHistoryButtons();
    SetStatus(L"Reset the editor to the current local lyric cache.");
}

void BeatDropLyricsEditor::FormatParsedText()
{
    if (!m_parsed)
        return;
    CHARRANGE saved = {};
    SendMessageW(m_parsed, EM_EXGETSEL, 0, reinterpret_cast<LPARAM>(&saved));
    const std::wstring text = Read(m_parsed);
    const CHARRANGE all = {0, static_cast<LONG>(text.size())};
    SendMessageW(m_parsed, EM_EXSETSEL, 0, reinterpret_cast<LPARAM>(&all));
    CHARFORMAT2W normal = {};
    normal.cbSize = sizeof(normal);
    normal.dwMask = CFM_COLOR | CFM_BOLD;
    normal.dwEffects = 0;
    normal.crTextColor = RGB(232, 232, 238);
    SendMessageW(m_parsed, EM_SETCHARFORMAT, SCF_SELECTION,
        reinterpret_cast<LPARAM>(&normal));

    CHARFORMAT2W timestamp = {};
    timestamp.cbSize = sizeof(timestamp);
    timestamp.dwMask = CFM_COLOR | CFM_BOLD;
    timestamp.dwEffects = CFE_BOLD;
    timestamp.crTextColor = RGB(244, 156, 64);
    LONG richTextCursor = 0;
    for (size_t cursor = 0; cursor < text.size(); )
    {
        const size_t open = text.find(L'[', cursor);
        if (open == std::wstring::npos)
            break;
        const size_t close = text.find(L']', open + 1);
        if (close == std::wstring::npos)
            break;
        double seconds = 0.0;
        if (BeatDropLrc::ParseTimestamp(text.substr(open + 1, close - open - 1), seconds))
        {
            const std::wstring token = text.substr(open, close - open + 1);
            FINDTEXTEXW found = {};
            found.chrg.cpMin = richTextCursor;
            found.chrg.cpMax = -1;
            found.lpstrText = const_cast<wchar_t*>(token.c_str());
            if (SendMessageW(m_parsed, EM_FINDTEXTEXW, FR_DOWN,
                reinterpret_cast<LPARAM>(&found)) >= 0)
            {
                SendMessageW(m_parsed, EM_EXSETSEL, 0,
                    reinterpret_cast<LPARAM>(&found.chrgText));
                SendMessageW(m_parsed, EM_SETCHARFORMAT, SCF_SELECTION,
                    reinterpret_cast<LPARAM>(&timestamp));
                richTextCursor = found.chrgText.cpMax;
            }
        }
        cursor = close + 1;
    }
    SendMessageW(m_parsed, EM_EXSETSEL, 0, reinterpret_cast<LPARAM>(&saved));
}

void BeatDropLyricsEditor::HandleEditorSelection()
{
    const int line = SelectedLine();
    if (line >= 0)
        m_selectedLine = line;
}
