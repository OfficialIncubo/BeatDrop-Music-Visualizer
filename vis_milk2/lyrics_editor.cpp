#include "lyrics_editor.h"

#include "lyrics_manager.h"
#include "songtitlegetter.h"

#include <algorithm>
#include <commctrl.h>
#include <commdlg.h>
#include <fstream>
#include <iomanip>
#include <cmath>
#include <array>
#include <bcrypt.h>
#include <cstdint>
#include <richedit.h>
#include <regex>
#include <winhttp.h>
#include <sstream>

extern SongTitleGetter songtitlegetter;

namespace
{
    constexpr int ID_PARSED = 1000;
    constexpr int ID_CLOSE = 1002;
    constexpr int ID_INPUT = 1003;
    constexpr int ID_PARSE = 1004;
    constexpr int ID_CAPTURE = 1005;
    constexpr int ID_SAVE = 1006;
    constexpr int ID_DELETE = 1007;
    constexpr int ID_RESET = 1008;
    constexpr int ID_MINUS5 = 1009;
    constexpr int ID_MINUS10 = 1010;
    constexpr int ID_PLUS10 = 1011;
    constexpr int ID_INSERT = 1012;
    constexpr int ID_UP = 1013;
    constexpr int ID_DOWN = 1014;
    constexpr int ID_UNDO = 1015;
    constexpr int ID_REDO = 1016;
    constexpr int ID_SEEK = 1017;
    constexpr int ID_IMPORT = 1019;
    constexpr int ID_STATUS = 1022;
    constexpr int ID_MINUS30 = 1030;
    constexpr int ID_PLUS30 = 1031;
    constexpr int ID_PLUS5 = 1101;
    const wchar_t* CLASS_NAME = L"BeatDropLyricsEditorWindow";

    std::string JsonEscape(const std::string& value)
    {
        std::string escaped;
        for (unsigned char c : value)
        {
            if (c == '\\') escaped += "\\\\";
            else if (c == '"') escaped += "\\\"";
            else if (c == '\n') escaped += "\\n";
            else if (c == '\r') escaped += "\\r";
            else if (c == '\t') escaped += "\\t";
            else if (c < 0x20) { char b[7] = {}; sprintf_s(b, "\\u%04x", c); escaped += b; }
            else escaped.push_back(static_cast<char>(c));
        }
        return escaped;
    }

    std::string JsonValue(const std::string& body, const char* key)
    {
        const std::string marker = std::string("\"") + key + "\"";
        size_t at = body.find(marker); if (at == std::string::npos) return {};
        at = body.find(':', at + marker.size()); at = at == std::string::npos ? at : body.find('"', at + 1);
        if (at == std::string::npos) return {};
        std::string result;
        for (++at; at < body.size(); ++at)
        {
            if (body[at] == '"') break;
            if (body[at] == '\\' && at + 1 < body.size()) { const char c = body[++at]; result.push_back(c == 'n' ? '\n' : c == 'r' ? '\r' : c == 't' ? '\t' : c); }
            else result.push_back(body[at]);
        }
        return result;
    }

    bool Sha256(const std::string& value, std::array<BYTE, 32>& digest)
    {
        BCRYPT_ALG_HANDLE algorithm = nullptr; BCRYPT_HASH_HANDLE hash = nullptr; DWORD objectLength = 0, resultLength = 0;
        bool ok = BCryptOpenAlgorithmProvider(&algorithm, BCRYPT_SHA256_ALGORITHM, nullptr, 0) == 0 && BCryptGetProperty(algorithm, BCRYPT_OBJECT_LENGTH, reinterpret_cast<PUCHAR>(&objectLength), sizeof(objectLength), &resultLength, 0) == 0;
        std::vector<BYTE> object(objectLength);
        if (ok) ok = BCryptCreateHash(algorithm, &hash, object.data(), objectLength, nullptr, 0, 0) == 0;
        if (ok) ok = BCryptHashData(hash, reinterpret_cast<PUCHAR>(const_cast<char*>(value.data())), static_cast<ULONG>(value.size()), 0) == 0;
        if (ok) ok = BCryptFinishHash(hash, digest.data(), static_cast<ULONG>(digest.size()), 0) == 0;
        if (hash) BCryptDestroyHash(hash); if (algorithm) BCryptCloseAlgorithmProvider(algorithm, 0); return ok;
    }

    bool HashMeetsTarget(const std::array<BYTE, 32>& hash, const std::string& target)
    {
        if (target.size() != 64) return false; const char hex[] = "0123456789abcdef";
        for (size_t i = 0; i < hash.size(); ++i) { const char high = hex[hash[i] >> 4], low = hex[hash[i] & 15]; const char eh = static_cast<char>(tolower(static_cast<unsigned char>(target[i * 2]))), el = static_cast<char>(tolower(static_cast<unsigned char>(target[i * 2 + 1]))); if (high < eh) return true; if (high > eh) return false; if (low < el) return true; if (low > el) return false; } return true;
    }

    std::string SolveChallenge(const std::string& prefix, const std::string& target)
    {
        for (uint64_t nonce = 0; nonce < 1000000000ULL; ++nonce) { const std::string candidate = prefix + std::to_string(nonce); std::array<BYTE, 32> hash = {}; if (Sha256(candidate, hash) && HashMeetsTarget(hash, target)) return std::to_string(nonce); }
        return {};
    }

    bool PostLrclib(const wchar_t* path, const std::string& body, const std::wstring& extraHeaders, DWORD& status, std::string& response)
    {
        HINTERNET session = WinHttpOpen(L"BeatDrop Music Visualizer/1.0", WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY, nullptr, nullptr, 0); if (!session) return false;
        HINTERNET connection = WinHttpConnect(session, L"lrclib.net", INTERNET_DEFAULT_HTTPS_PORT, 0); HINTERNET request = connection ? WinHttpOpenRequest(connection, L"POST", path, nullptr, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE) : nullptr;
        const std::wstring headers = L"Content-Type: application/json\r\nAccept: application/json\r\n" + extraHeaders; bool ok = request && WinHttpSendRequest(request, headers.c_str(), static_cast<DWORD>(headers.size()), const_cast<char*>(body.data()), static_cast<DWORD>(body.size()), static_cast<DWORD>(body.size()), 0) && WinHttpReceiveResponse(request, nullptr);
        if (ok) { DWORD size = sizeof(status); WinHttpQueryHeaders(request, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER, WINHTTP_HEADER_NAME_BY_INDEX, &status, &size, WINHTTP_NO_HEADER_INDEX); DWORD available = 0; while (WinHttpQueryDataAvailable(request, &available) && available) { std::string chunk(available, '\0'); DWORD read = 0; if (!WinHttpReadData(request, chunk.data(), available, &read) || !read) break; response.append(chunk.data(), read); } }
        if (request) WinHttpCloseHandle(request); if (connection) WinHttpCloseHandle(connection); WinHttpCloseHandle(session); return ok;
    }

    HBRUSH WindowBrush() { static HBRUSH b = CreateSolidBrush(RGB(35, 35, 35)); return b; }
    HBRUSH PaneBrush() { static HBRUSH b = CreateSolidBrush(RGB(43, 43, 43)); return b; }
    HFONT EditorFont()
    {
        static HFONT f = CreateFontW(-14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
            DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
        return f;
    }
    HFONT UndoFont()
    {
        static HFONT f = CreateFontW(-17, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
            DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI Symbol");
        return f;
    }
    void DarkTheme(HWND control)
    {
        if (HMODULE theme = LoadLibraryW(L"uxtheme.dll"))
        {
            using SetTheme = HRESULT(WINAPI*)(HWND, LPCWSTR, LPCWSTR);
            auto fn = reinterpret_cast<SetTheme>(GetProcAddress(theme, "SetWindowTheme"));
            if (fn) fn(control, L"DarkMode_Explorer", nullptr);
        }
    }
    void RoundButton(HWND button, int width)
    {
        if (button) SetWindowRgn(button, CreateRoundRectRgn(0, 0, width + 1, 33, 16, 16), TRUE);
    }
    void Tooltip(HWND tip, HWND control, const wchar_t* text)
    {
        if (!tip || !control) return;
        TTTOOLINFOW info = {};
        info.cbSize = sizeof(info); info.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
        info.hwnd = GetParent(control); info.uId = reinterpret_cast<UINT_PTR>(control);
        info.lpszText = const_cast<wchar_t*>(text);
        SendMessageW(tip, TTM_ADDTOOLW, 0, reinterpret_cast<LPARAM>(&info));
    }
}

void BeatDropLyricsEditor::Open(HWND owner, BeatDropLyricsManager* manager,
                                const std::wstring& artist, const std::wstring& title)
{
    m_manager = manager; m_artist = artist; m_title = title;
    if (!m_hwnd)
    {
        WNDCLASSW wc = {};
        wc.lpfnWndProc = WindowProc; wc.hInstance = GetModuleHandleW(nullptr);
        wc.hCursor = LoadCursorW(nullptr, MAKEINTRESOURCEW(IDC_ARROW)); wc.hbrBackground = WindowBrush();
        wc.lpszClassName = CLASS_NAME; RegisterClassW(&wc);
        m_hwnd = CreateWindowExW(WS_EX_APPWINDOW, CLASS_NAME, L"BeatDrop Lyrics Editor",
            WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, 1160, 720,
            owner, nullptr, wc.hInstance, this);
    }
    else
    {
        SetWindowTextW(m_hwnd, (L"BeatDrop Lyrics Editor - " + m_artist + L" - " + m_title).c_str());
        const std::wstring lrc = m_manager ? m_manager->CurrentLrc() : L"";
        Write(m_input, lrc); Write(m_parsed, lrc); m_undo.clear(); m_redo.clear();
        ShowWindow(m_hwnd, SW_SHOWNORMAL); SetForegroundWindow(m_hwnd);
    }
}

LRESULT CALLBACK BeatDropLyricsEditor::WindowProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    auto* self = reinterpret_cast<BeatDropLyricsEditor*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    if (msg == WM_NCCREATE)
    {
        self = reinterpret_cast<BeatDropLyricsEditor*>(reinterpret_cast<CREATESTRUCTW*>(lp)->lpCreateParams);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self)); self->m_hwnd = hwnd;
    }
    return self ? self->HandleMessage(msg, wp, lp) : DefWindowProcW(hwnd, msg, wp, lp);
}

LRESULT CALLBACK BeatDropLyricsEditor::ParsedEditProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    auto* self = reinterpret_cast<BeatDropLyricsEditor*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    if (self && (msg == WM_PASTE || msg == WM_CUT))
        self->PushUndo();
    if (self && msg == WM_KEYDOWN && wp != VK_DELETE && wp != VK_SPACE &&
        wp != VK_LEFT && wp != VK_RIGHT && wp != VK_UP && wp != VK_DOWN &&
        wp != VK_HOME && wp != VK_END && wp != VK_PRIOR && wp != VK_NEXT &&
        !(GetKeyState(VK_CONTROL) & 0x8000) && !(GetKeyState(VK_MENU) & 0x8000))
        self->PushUndo();
    if (self && msg == WM_KEYDOWN && wp == VK_DELETE)
    {
        self->HandleParsedKey(wp);
        return 0;
    }
    if (self && msg == WM_KEYDOWN && wp == VK_SPACE)
    {
        self->CaptureAndAdvance();
        return 0;
    }
    if (self && (msg == WM_LBUTTONUP || msg == WM_KEYUP))
    {
        const LRESULT result = self->m_oldParsedProc
            ? CallWindowProcW(self->m_oldParsedProc, hwnd, msg, wp, lp)
            : DefWindowProcW(hwnd, msg, wp, lp);
        self->HandleEditorSelection();
        return result;
    }
    return self && self->m_oldParsedProc
        ? CallWindowProcW(self->m_oldParsedProc, hwnd, msg, wp, lp)
        : DefWindowProcW(hwnd, msg, wp, lp);
}

LRESULT BeatDropLyricsEditor::HandleMessage(UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg)
    {
    case WM_CREATE:
    {
        LoadLibraryW(L"Msftedit.dll");
        m_input = CreateWindowExW(0, L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_WANTRETURN, 0, 0, 0, 0, m_hwnd, (HMENU)ID_INPUT, nullptr, nullptr);
        m_parsed = CreateWindowExW(0, MSFTEDIT_CLASS, L"", WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_WANTRETURN, 0, 0, 0, 0, m_hwnd, (HMENU)ID_PARSED, nullptr, nullptr);
        SetWindowLongPtrW(m_parsed, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
        m_oldParsedProc = reinterpret_cast<WNDPROC>(SetWindowLongPtrW(m_parsed, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(ParsedEditProc)));
        const DWORD style = WS_CHILD | WS_VISIBLE | BS_OWNERDRAW;
        const auto button = [&](const wchar_t* text, int id) { return CreateWindowW(L"BUTTON", text, style, 0, 0, 1, 1, m_hwnd, (HMENU)id, nullptr, nullptr); };
        button(L"Parse text", ID_PARSE); button(L"Import LRC", ID_IMPORT); button(L"Delete line", ID_DELETE); button(L"Insert silence", ID_INSERT); button(L"Reset editor", ID_RESET); button(L"Undo", ID_UNDO); button(L"Redo", ID_REDO); button(L"Seek to line", ID_SEEK); button(L"Upload to LRCLIB", 1018);
        button(L"\x25B2", ID_UP); button(L"\x25BC", ID_DOWN); button(L"Capture timestamp", ID_CAPTURE); button(L"-30s", ID_MINUS30); button(L"-10s", ID_MINUS10); button(L"-5s", ID_MINUS5); button(L"+5s", ID_PLUS5); button(L"+10s", ID_PLUS10); button(L"+30s", ID_PLUS30); button(L"Save local LRC", ID_SAVE); button(L"Close", ID_CLOSE);
        CreateWindowW(L"STATIC", L"Input", WS_CHILD | WS_VISIBLE, 0, 0, 1, 1, m_hwnd, (HMENU)1020, nullptr, nullptr);
        CreateWindowW(L"STATIC", L"Parsed Text", WS_CHILD | WS_VISIBLE, 0, 0, 1, 1, m_hwnd, (HMENU)1021, nullptr, nullptr);
        m_status = CreateWindowW(L"STATIC", L"Edit timed lyrics, then save them to the local cache.", WS_CHILD | WS_VISIBLE, 0, 0, 1, 1, m_hwnd, (HMENU)ID_STATUS, nullptr, nullptr);
        for (int id : {ID_PARSE, ID_IMPORT, ID_DELETE, ID_INSERT, ID_RESET, ID_UNDO, ID_REDO, ID_SEEK, ID_UP, ID_DOWN, ID_CAPTURE, ID_MINUS30, ID_MINUS10, ID_MINUS5, ID_PLUS5, ID_PLUS10, ID_PLUS30, ID_SAVE, ID_CLOSE}) RoundButton(GetDlgItem(m_hwnd, id), 120);
        for (HWND c : {m_input, m_parsed, GetDlgItem(m_hwnd, 1020), GetDlgItem(m_hwnd, 1021), m_status}) { SendMessageW(c, WM_SETFONT, (WPARAM)EditorFont(), TRUE); DarkTheme(c); }
        if (HWND tooltip = CreateWindowExW(WS_EX_TOPMOST, TOOLTIPS_CLASSW, nullptr,
            WS_POPUP | TTS_ALWAYSTIP | TTS_NOPREFIX, CW_USEDEFAULT, CW_USEDEFAULT,
            CW_USEDEFAULT, CW_USEDEFAULT, m_hwnd, nullptr, GetModuleHandleW(nullptr), nullptr))
        {
            SendMessageW(tooltip, TTM_SETMAXTIPWIDTH, 0, 420);
            Tooltip(tooltip, GetDlgItem(m_hwnd, ID_PARSE), L"Parse the Input pane into editable Parsed Text.");
            Tooltip(tooltip, GetDlgItem(m_hwnd, ID_INSERT), L"Insert a timestamp-only silence line five seconds after the selected line.");
            Tooltip(tooltip, GetDlgItem(m_hwnd, ID_SEEK), L"Seek the active Windows media session to the selected line.");
            Tooltip(tooltip, GetDlgItem(m_hwnd, ID_CAPTURE), L"Stamp the selected line with the current playback position and advance.");
        }
        SendMessageW(m_parsed, EM_SETBKGNDCOLOR, 0, RGB(43, 43, 43));
        Write(m_input, m_manager ? m_manager->CurrentLrc() : L""); Write(m_parsed, Read(m_input)); FormatParsedText(); m_selectedLine = 0; SetTimer(m_hwnd, 1, 500, nullptr);
        ResizeControls(); return 0;
    }
    case WM_SIZE: ResizeControls(); return 0;
    case WM_TIMER: if (m_status && m_manager) SetWindowTextW(m_status, m_manager->Status().c_str()); return 0;
    case WM_ERASEBKGND: { RECT r; GetClientRect(m_hwnd, &r); FillRect((HDC)wp, &r, WindowBrush()); return 1; }
    case WM_CTLCOLORSTATIC: { HDC dc = (HDC)wp; SetTextColor(dc, RGB(224,224,224)); SetBkColor(dc, RGB(35,35,35)); return (LRESULT)WindowBrush(); }
    case WM_CTLCOLOREDIT: { HDC dc = (HDC)wp; SetBkColor(dc, RGB(43,43,43)); if ((HWND)lp != m_parsed) SetTextColor(dc, RGB(224,224,224)); return (LRESULT)PaneBrush(); }
    case WM_DRAWITEM:
    {
        auto* item = reinterpret_cast<DRAWITEMSTRUCT*>(lp); if (!item) return 0;
        const bool primary = item->CtlID == ID_SAVE; const bool pressed = (item->itemState & ODS_SELECTED) != 0;
        HBRUSH fill = CreateSolidBrush(primary ? (pressed ? RGB(180,88,18) : RGB(214,104,22)) : (pressed ? RGB(70,70,70) : RGB(53,53,53)));
        FillRect(item->hDC, &item->rcItem, WindowBrush()); SelectObject(item->hDC, fill); RoundRect(item->hDC, item->rcItem.left, item->rcItem.top, item->rcItem.right, item->rcItem.bottom, 16, 16); DeleteObject(fill);
        SetBkMode(item->hDC, TRANSPARENT); SetTextColor(item->hDC, RGB(235,235,235));
        wchar_t text[128] = {}; GetWindowTextW(item->hwndItem, text, 128);
        if (item->CtlID == ID_UNDO || item->CtlID == ID_REDO)
        {
            SelectObject(item->hDC, UndoFont());
            const int cx = item->rcItem.left + 22;
            const int cy = (item->rcItem.top + item->rcItem.bottom) / 2;
            Arc(item->hDC, cx - 9, cy - 9, cx + 9, cy + 9,
                item->CtlID == ID_UNDO ? cx - 9 : cx + 9, cy - 4,
                item->CtlID == ID_UNDO ? cx + 4 : cx - 4, cy - 9);
            POINT arrow[3] = {};
            if (item->CtlID == ID_UNDO)
            {
                arrow[0] = {cx - 9, cy - 4}; arrow[1] = {cx - 2, cy - 7}; arrow[2] = {cx - 4, cy + 1};
            }
            else
            {
                arrow[0] = {cx + 9, cy - 4}; arrow[1] = {cx + 2, cy - 7}; arrow[2] = {cx + 4, cy + 1};
            }
            Polygon(item->hDC, arrow, 3);
            RECT label = item->rcItem; label.left += 34;
            SelectObject(item->hDC, EditorFont()); DrawTextW(item->hDC, text, -1, &label, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
        }
        else
        {
            SelectObject(item->hDC, EditorFont()); DrawTextW(item->hDC, text, -1, &item->rcItem, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        }
        return 1;
    }
    case WM_COMMAND:
        switch (LOWORD(wp)) { case ID_PARSE: ParseText(); break; case ID_IMPORT: Import(); break; case ID_SAVE: Save(); break; case ID_CAPTURE: CaptureTimestamp(); break; case ID_INSERT: InsertSilence(); break; case ID_DELETE: DeleteSelectedLine(); break; case ID_UP: MoveSelectedLine(-1); break; case ID_DOWN: MoveSelectedLine(1); break; case ID_UNDO: Undo(); break; case ID_REDO: Redo(); break; case ID_RESET: Reset(); break; case ID_SEEK: SeekToLine(); break; case 1018: Upload(); break; case ID_MINUS30: songtitlegetter.SeekRelative(-30.0); break; case ID_MINUS10: songtitlegetter.SeekRelative(-10.0); break; case ID_MINUS5: songtitlegetter.SeekRelative(-5.0); break; case ID_PLUS5: songtitlegetter.SeekRelative(5.0); break; case ID_PLUS10: songtitlegetter.SeekRelative(10.0); break; case ID_PLUS30: songtitlegetter.SeekRelative(30.0); break; case ID_CLOSE: DestroyWindow(m_hwnd); break; } return 0;
    case WM_CLOSE: DestroyWindow(m_hwnd); return 0;
    case WM_DESTROY:
        KillTimer(m_hwnd, 1);
        if (m_parsed && m_oldParsedProc) SetWindowLongPtrW(m_parsed, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(m_oldParsedProc));
        m_oldParsedProc = nullptr; m_hwnd = m_input = m_parsed = m_status = m_position = nullptr; return 0;
    }
    return DefWindowProcW(m_hwnd, msg, wp, lp);
}

void BeatDropLyricsEditor::ResizeControls()
{
    if (!m_hwnd) return; RECT r = {}; GetClientRect(m_hwnd, &r); const int w = r.right, h = r.bottom; const int paneY = 74, bottom = h - 44; const int half = (std::max)(260, (w - 24) / 2);
    MoveWindow(m_input, 8, paneY, half - 4, (std::max)(100, bottom - paneY - 8), TRUE); MoveWindow(m_parsed, 16 + half, paneY, half - 36, (std::max)(100, bottom - paneY - 8), TRUE);
    MoveWindow(GetDlgItem(m_hwnd, 1020), 8, 52, half, 18, TRUE); MoveWindow(GetDlgItem(m_hwnd, 1021), 16 + half, 52, half, 18, TRUE);
    const int arrowX = 16 + half + half - 30;
    MoveWindow(GetDlgItem(m_hwnd, ID_UP), arrowX, paneY + 8, 28, 32, TRUE);
    MoveWindow(GetDlgItem(m_hwnd, ID_DOWN), arrowX, paneY + 48, 28, 32, TRUE);
    RoundButton(GetDlgItem(m_hwnd, ID_UP), 28); RoundButton(GetDlgItem(m_hwnd, ID_DOWN), 28);
    int x = 8; const auto p = [&](int id, int width) { MoveWindow(GetDlgItem(m_hwnd, id), x, 8, width, 32, TRUE); RoundButton(GetDlgItem(m_hwnd, id), width); x += width + 5; };
    p(ID_PARSE, 100); p(ID_IMPORT, 100); p(ID_DELETE, 100); p(ID_INSERT, 110); p(ID_RESET, 105); p(ID_UNDO, 90); p(ID_REDO, 90); p(ID_SEEK, 110); p(1018, 140);
    x = 8; const auto q = [&](int id, int width) { MoveWindow(GetDlgItem(m_hwnd, id), x, bottom + 4, width, 32, TRUE); RoundButton(GetDlgItem(m_hwnd, id), width); x += width + 4; };
    q(ID_CAPTURE, 130); q(ID_MINUS30, 48); q(ID_MINUS10, 48); q(ID_MINUS5, 48); q(ID_PLUS5, 48); q(ID_PLUS10, 52); q(ID_PLUS30, 52);
    const int saveX = w - 230;
    MoveWindow(m_status, x + 140, bottom + 8, (std::max)(100, saveX - x - 150), 24, TRUE);
    MoveWindow(GetDlgItem(m_hwnd, ID_SAVE), saveX, bottom + 4, 130, 32, TRUE);
    MoveWindow(GetDlgItem(m_hwnd, ID_CLOSE), w - 92, bottom + 4, 84, 32, TRUE);
}

std::wstring BeatDropLyricsEditor::Read(HWND c) const { int n = GetWindowTextLengthW(c); std::wstring s(n + 1, L'\0'); GetWindowTextW(c, s.data(), n + 1); s.resize(n); return s; }
void BeatDropLyricsEditor::Write(HWND c, const std::wstring& s)
{
    if (!c) return;
    std::wstring value = s;
    if (c == m_input)
    {
        std::wstring normalized;
        normalized.reserve(value.size() + 16);
        for (size_t i = 0; i < value.size(); ++i)
        {
            if (value[i] == L'\r')
            {
                normalized.push_back(L'\r');
                if (i + 1 >= value.size() || value[i + 1] != L'\n') normalized.push_back(L'\n');
            }
            else if (value[i] == L'\n')
            {
                if (i == 0 || value[i - 1] != L'\r') normalized.push_back(L'\r');
                normalized.push_back(L'\n');
            }
            else normalized.push_back(value[i]);
        }
        value.swap(normalized);
    }
    SetWindowTextW(c, value.c_str());
}
void BeatDropLyricsEditor::SetStatus(const std::wstring& s) { if (m_status) SetWindowTextW(m_status, s.c_str()); }
void BeatDropLyricsEditor::PushUndo() { m_undo.push_back(Read(m_parsed)); if (m_undo.size() > 50) m_undo.erase(m_undo.begin()); m_redo.clear(); }
void BeatDropLyricsEditor::ParseText() { PushUndo(); Write(m_parsed, Read(m_input)); FormatParsedText(); SetStatus(L"Parsed input into the editable timed-lyrics pane."); }
void BeatDropLyricsEditor::Reset() { Write(m_input, m_manager ? m_manager->CurrentLrc() : L""); Write(m_parsed, Read(m_input)); FormatParsedText(); m_undo.clear(); m_redo.clear(); SetStatus(L"Editor reset to the current cached lyrics."); }
void BeatDropLyricsEditor::Save()
{
    if (!m_manager) return;
    const std::wstring source = Read(m_parsed);
    std::wostringstream metadata;
    metadata << L"[duration:" << std::fixed << std::setprecision(2) << songtitlegetter.currentDurationSeconds << L"]\r\n"
             << L"[ar:" << m_artist << L"]\r\n"
             << L"[al:" << songtitlegetter.currentAlbum << L"]\r\n"
             << L"[ti:" << m_title << L"]\r\n"
             << L"[length:" << std::fixed << std::setprecision(2) << songtitlegetter.currentDurationSeconds << L"]\r\n"
             << L"[by:BeatDrop Music Visualizer]\r\n[re:BeatDrop Music Visualizer]\r\n[ve:1.0]\r\n[offset:0]\r\n\r\n"
             << source;
    m_manager->SaveCurrentLrc(metadata.str());
    SetStatus(L"Saved metadata-complete LRC to the BeatDrop local cache.");
}
void BeatDropLyricsEditor::Upload()
{
    if (!m_parsed) return;
    const int warning = MessageBoxW(m_hwnd,
        L"LRCLIB requires a proof-of-work calculation before publishing.\n\nContinue with upload?",
        L"Publish lyrics to LRCLIB", MB_YESNO | MB_ICONINFORMATION);
    if (warning != IDYES) return;
    const std::string synced = BeatDropLyricsManager::Utf8(Read(m_parsed));
    const std::string artist = JsonEscape(BeatDropLyricsManager::Utf8(m_artist));
    const std::string title = JsonEscape(BeatDropLyricsManager::Utf8(m_title));
    const std::string album = JsonEscape(BeatDropLyricsManager::Utf8(songtitlegetter.currentAlbum));
    const std::string plain = JsonEscape(std::regex_replace(synced, std::regex(R"(\[[0-9]{1,3}:[0-9]{2}(?:\.[0-9]{1,3})?\])"), ""));
    const std::string payload = "{\"trackName\":\"" + title + "\",\"artistName\":\"" + artist + "\",\"albumName\":\"" + album + "\",\"duration\":" + std::to_string(songtitlegetter.currentDurationSeconds) + ",\"plainLyrics\":\"" + plain + "\",\"syncedLyrics\":\"" + JsonEscape(synced) + "\"}";
    DWORD status = 0; std::string challenge;
    if (!PostLrclib(L"/api/request-challenge", "{}", L"", status, challenge)) { MessageBoxW(m_hwnd, L"Unable to request an LRCLIB challenge.", L"LRCLIB upload", MB_OK | MB_ICONERROR); return; }
    const std::string prefix = JsonValue(challenge, "prefix"); const std::string target = JsonValue(challenge, "target"); const std::string nonce = SolveChallenge(prefix, target);
    if (nonce.empty()) { MessageBoxW(m_hwnd, L"Unable to solve the LRCLIB challenge.", L"LRCLIB upload", MB_OK | MB_ICONERROR); return; }
    std::wstring token = L"X-Publish-Token: " + std::wstring(prefix.begin(), prefix.end()) + L":" + std::wstring(nonce.begin(), nonce.end()) + L"\r\n";
    std::string response; if (!PostLrclib(L"/api/publish", payload, token, status, response) || status < 200 || status >= 300) { MessageBoxW(m_hwnd, (L"LRCLIB rejected the upload (HTTP " + std::to_wstring(status) + L").").c_str(), L"LRCLIB upload", MB_OK | MB_ICONERROR); return; }
    SetStatus(L"Lyrics uploaded to LRCLIB."); MessageBoxW(m_hwnd, L"Lyrics uploaded to LRCLIB.", L"LRCLIB upload", MB_OK | MB_ICONINFORMATION);
}
void BeatDropLyricsEditor::Undo()
{
    if (!m_parsed || m_undo.empty()) { SetStatus(L"Nothing to undo."); return; }
    m_redo.push_back(Read(m_parsed)); Write(m_parsed, m_undo.back()); m_undo.pop_back(); FormatParsedText(); SetFocus(m_parsed);
}
void BeatDropLyricsEditor::Redo()
{
    if (!m_parsed || m_redo.empty()) { SetStatus(L"Nothing to redo."); return; }
    m_undo.push_back(Read(m_parsed)); Write(m_parsed, m_redo.back()); m_redo.pop_back(); FormatParsedText(); SetFocus(m_parsed);
}
void BeatDropLyricsEditor::CaptureTimestamp()
{
    if (!m_parsed) return;
    PushUndo();
    CHARRANGE selection = {};
    SendMessageW(m_parsed, EM_EXGETSEL, 0, reinterpret_cast<LPARAM>(&selection));
    const LONG targetLine = static_cast<LONG>(SendMessageW(m_parsed, EM_LINEFROMCHAR, selection.cpMin, 0));
    std::wstring document = Read(m_parsed);
    if (document.empty() || targetLine < 0) return;

    size_t lineStart = 0;
    for (LONG index = 0; index < targetLine && lineStart < document.size(); ++index)
    {
        const size_t newline = document.find(L'\n', lineStart);
        if (newline == std::wstring::npos) { lineStart = document.size(); break; }
        lineStart = newline + 1;
    }
    if (lineStart >= document.size()) return;
    size_t lineEnd = document.find_first_of(L"\r\n", lineStart);
    if (lineEnd == std::wstring::npos) lineEnd = document.size();

    const double position = (std::max)(0.0, songtitlegetter.currentPositionSeconds);
    const int centiseconds = (std::max)(0, static_cast<int>(std::lround(position * 100.0)));
    const int minutes = centiseconds / 6000;
    const int remainder = centiseconds % 6000;
    std::wostringstream timestamp;
    timestamp << L"[" << std::setfill(L'0') << std::setw(2) << minutes << L":"
              << std::setw(2) << remainder / 100 << L"." << std::setw(2) << remainder % 100 << L"] ";

    std::wstring lineText = document.substr(lineStart, lineEnd - lineStart);
    const std::wregex existingStamp(LR"(^\[[0-9]+:[0-9]{2}(?:\.[0-9]{1,3})?\])");
    lineText = std::regex_replace(lineText, existingStamp, timestamp.str(), std::regex_constants::format_first_only);
    if (lineText == document.substr(lineStart, lineEnd - lineStart)) lineText.insert(0, timestamp.str());
    document.replace(lineStart, lineEnd - lineStart, lineText);

    Write(m_parsed, document);
    FormatParsedText();
    const LONG updatedStart = static_cast<LONG>(SendMessageW(m_parsed, EM_LINEINDEX, targetLine, 0));
    const LONG updatedLength = updatedStart >= 0 ? static_cast<LONG>(SendMessageW(m_parsed, EM_LINELENGTH, updatedStart, 0)) : 0;
    CHARRANGE updatedSelection{updatedStart, updatedStart + (std::max)(0L, updatedLength)};
    SendMessageW(m_parsed, EM_EXSETSEL, 0, reinterpret_cast<LPARAM>(&updatedSelection));
    SetFocus(m_parsed);
    SetStatus(L"Captured the current Windows Now Playing timestamp on the selected line.");
}

void BeatDropLyricsEditor::CaptureAndAdvance()
{
    if (!m_parsed) return;
    CHARRANGE selection = {}; SendMessageW(m_parsed, EM_EXGETSEL, 0, reinterpret_cast<LPARAM>(&selection));
    const LONG current = m_selectedLine >= 0 ? m_selectedLine : static_cast<LONG>(SendMessageW(m_parsed, EM_LINEFROMCHAR, selection.cpMin, 0));
    m_selectedLine = static_cast<int>(current); CaptureTimestamp();
    const LONG next = current + 1; const LONG start = static_cast<LONG>(SendMessageW(m_parsed, EM_LINEINDEX, next, 0));
    if (start < 0) return;
    const LONG length = static_cast<LONG>(SendMessageW(m_parsed, EM_LINELENGTH, start, 0)); CHARRANGE nextRange{start, start + length};
    SendMessageW(m_parsed, EM_EXSETSEL, 0, reinterpret_cast<LPARAM>(&nextRange)); m_selectedLine = static_cast<int>(next); SetFocus(m_parsed);
}

void BeatDropLyricsEditor::HandleEditorSelection()
{
    if (!m_parsed) return;
    CHARRANGE selection = {}; SendMessageW(m_parsed, EM_EXGETSEL, 0, reinterpret_cast<LPARAM>(&selection));
    if (selection.cpMin != selection.cpMax) return;
    const LONG line = static_cast<LONG>(SendMessageW(m_parsed, EM_LINEFROMCHAR, selection.cpMin, 0));
    const LONG start = static_cast<LONG>(SendMessageW(m_parsed, EM_LINEINDEX, line, 0));
    const LONG length = start >= 0 ? static_cast<LONG>(SendMessageW(m_parsed, EM_LINELENGTH, start, 0)) : 0;
    if (start < 0 || length <= 0) return;
    m_selectedLine = static_cast<int>(line);
    CHARRANGE whole{start, start + length}; SendMessageW(m_parsed, EM_EXSETSEL, 0, reinterpret_cast<LPARAM>(&whole));
}
void BeatDropLyricsEditor::SeekToLine()
{
    if (!m_parsed) return;
    CHARRANGE selection = {}; SendMessageW(m_parsed, EM_EXGETSEL, 0, reinterpret_cast<LPARAM>(&selection));
    const LONG line = static_cast<LONG>(SendMessageW(m_parsed, EM_LINEFROMCHAR, selection.cpMin, 0));
    const LONG start = static_cast<LONG>(SendMessageW(m_parsed, EM_LINEINDEX, line, 0));
    const LONG length = start >= 0 ? static_cast<LONG>(SendMessageW(m_parsed, EM_LINELENGTH, start, 0)) : 0;
    if (start < 0 || length <= 0) { SetStatus(L"Select a timestamped line first."); return; }
    std::wstring lineText(static_cast<size_t>(length), L' ');
    TEXTRANGEW range = {{start, start + length}, lineText.data()};
    SendMessageW(m_parsed, EM_GETTEXTRANGE, 0, reinterpret_cast<LPARAM>(&range));
    std::wsmatch match;
    const std::wregex timestamp(LR"(^\[([0-9]+):([0-9]{2})(?:\.([0-9]{1,3}))?\])");
    if (!std::regex_search(lineText, match, timestamp)) { SetStatus(L"Select a timestamped line first."); return; }
    try
    {
        const int minutes = std::stoi(match[1].str());
        double seconds = std::stod(match[2].str());
        if (match[3].matched) seconds += std::stod(L"0." + match[3].str());
        SetStatus(songtitlegetter.SeekTo(minutes * 60.0 + seconds) ? L"Seeked to the selected lyric line." : L"The active player does not support seeking.");
    }
    catch (...) { SetStatus(L"Select a valid LRC timestamped line first."); }
}
void BeatDropLyricsEditor::InsertSilence()
{
    if (!m_parsed) return;
    CHARRANGE selection = {}; SendMessageW(m_parsed, EM_EXGETSEL, 0, reinterpret_cast<LPARAM>(&selection));
    LONG line = m_selectedLine >= 0 ? m_selectedLine : static_cast<LONG>(SendMessageW(m_parsed, EM_LINEFROMCHAR, selection.cpMin, 0));
    const LONG start = static_cast<LONG>(SendMessageW(m_parsed, EM_LINEINDEX, line, 0));
    const LONG length = start >= 0 ? static_cast<LONG>(SendMessageW(m_parsed, EM_LINELENGTH, start, 0)) : 0;
    if (start < 0 || length < 0) return;
    std::wstring lineText(static_cast<size_t>(length) + 1, L' ');
    TEXTRANGEW range = {{start, start + length}, lineText.data()}; SendMessageW(m_parsed, EM_GETTEXTRANGE, 0, reinterpret_cast<LPARAM>(&range)); lineText.resize(length);
    std::wsmatch match; const std::wregex stamp(LR"(^\[([0-9]+):([0-9]{2})(?:\.([0-9]{1,3}))?\])");
    if (!std::regex_search(lineText, match, stamp)) return;
    const double seconds = std::stod(match[1].str()) * 60.0 + std::stod(match[2].str()) + (match[3].matched ? std::stod(L"0." + match[3].str()) : 0.0);
    const int cs = static_cast<int>(std::lround((seconds + 5.0) * 100.0));
    std::wostringstream formatted; formatted << L"[" << std::setfill(L'0') << std::setw(2) << cs / 6000 << L":" << std::setw(2) << (cs % 6000) / 100 << L"." << std::setw(2) << cs % 100 << L"]";
    const LONG nextStart = static_cast<LONG>(SendMessageW(m_parsed, EM_LINEINDEX, line + 1, 0));
    const bool hasNext = nextStart >= 0; CHARRANGE replacement{start, hasNext ? nextStart : start + length};
    SendMessageW(m_parsed, EM_EXSETSEL, 0, reinterpret_cast<LPARAM>(&replacement));
    const std::wstring inserted = lineText + L"\r\n" + formatted.str() + (hasNext ? L"\r\n" : L"");
    SendMessageW(m_parsed, EM_REPLACESEL, TRUE, reinterpret_cast<LPARAM>(inserted.c_str())); FormatParsedText(); m_selectedLine = static_cast<int>(line + 1); SetFocus(m_parsed);
}
void BeatDropLyricsEditor::DeleteSelectedLine() { PushUndo(); DWORD start = 0, end = 0; SendMessageW(m_parsed, EM_GETSEL, (WPARAM)&start, (LPARAM)&end); std::wstring s = Read(m_parsed); size_t a = s.rfind(L'\n', start ? start - 1 : 0); size_t b = s.find(L'\n', end); a = a == std::wstring::npos ? 0 : a + 1; b = b == std::wstring::npos ? s.size() : b + 1; s.erase(a, b - a); Write(m_parsed, s); FormatParsedText(); SetStatus(L"Removed the selected lyric line."); }
void BeatDropLyricsEditor::MoveSelectedLine(int direction)
{
    if (!m_parsed || direction == 0) return;
    CHARRANGE selection = {}; SendMessageW(m_parsed, EM_EXGETSEL, 0, reinterpret_cast<LPARAM>(&selection));
    const LONG current = m_selectedLine >= 0 ? m_selectedLine : static_cast<LONG>(SendMessageW(m_parsed, EM_LINEFROMCHAR, selection.cpMin, 0));
    std::wstring document = Read(m_parsed); std::vector<std::wstring> rows; std::wistringstream input(document); std::wstring row;
    while (std::getline(input, row)) { if (!row.empty() && row.back() == L'\r') row.pop_back(); rows.push_back(row); }
    const LONG target = current + direction;
    if (current < 0 || target < 0 || current >= static_cast<LONG>(rows.size()) || target >= static_cast<LONG>(rows.size())) return;
    std::swap(rows[static_cast<size_t>(current)], rows[static_cast<size_t>(target)]);
    std::wostringstream output; for (size_t i = 0; i < rows.size(); ++i) { if (i) output << L"\r\n"; output << rows[i]; }
    if (document.size() >= 2 && document.substr(document.size() - 2) == L"\r\n") output << L"\r\n";
    Write(m_parsed, output.str()); FormatParsedText(); m_selectedLine = static_cast<int>(target);
    const LONG start = static_cast<LONG>(SendMessageW(m_parsed, EM_LINEINDEX, target, 0)); const LONG length = start >= 0 ? static_cast<LONG>(SendMessageW(m_parsed, EM_LINELENGTH, start, 0)) : 0; CHARRANGE moved{start, start + (std::max)(0L, length)}; SendMessageW(m_parsed, EM_EXSETSEL, 0, reinterpret_cast<LPARAM>(&moved)); SetFocus(m_parsed);
}
void BeatDropLyricsEditor::Import()
{
    wchar_t path[32768] = {}; OPENFILENAMEW ofn = {}; ofn.lStructSize = sizeof(ofn); ofn.hwndOwner = m_hwnd; ofn.lpstrFilter = L"LRC lyrics\0*.lrc\0All files\0*.*\0"; ofn.lpstrFile = path; ofn.nMaxFile = 32768; ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
    if (!GetOpenFileNameW(&ofn)) return; std::ifstream file(path, std::ios::binary); if (!file) return; std::string bytes((std::istreambuf_iterator<char>(file)), {}); int n = MultiByteToWideChar(CP_UTF8, 0, bytes.data(), (int)bytes.size(), nullptr, 0); std::wstring text(n, L'\0'); MultiByteToWideChar(CP_UTF8, 0, bytes.data(), (int)bytes.size(), text.data(), n); Write(m_input, text); Write(m_parsed, text); FormatParsedText(); SetStatus(L"Imported LRC into both editor panes.");
}

void BeatDropLyricsEditor::FormatParsedText()
{
    if (!m_parsed) return;
    const std::wstring text = Read(m_parsed);
    const LONG length = static_cast<LONG>(text.size());
    CHARRANGE all{0, length};
    SendMessageW(m_parsed, EM_EXSETSEL, 0, reinterpret_cast<LPARAM>(&all));
    CHARFORMAT2W normal = {};
    normal.cbSize = sizeof(normal); normal.dwMask = CFM_BOLD | CFM_COLOR; normal.crTextColor = RGB(224, 224, 224);
    SendMessageW(m_parsed, EM_SETCHARFORMAT, SCF_SELECTION, reinterpret_cast<LPARAM>(&normal));

    const std::wregex timestamp(LR"(\[[0-9]{1,3}:[0-9]{2}(?:\.[0-9]{1,3})?\])");
    CHARFORMAT2W stamp = {};
    stamp.cbSize = sizeof(stamp); stamp.dwMask = CFM_BOLD | CFM_COLOR;
    stamp.dwEffects = CFE_BOLD; stamp.crTextColor = RGB(230, 125, 25);
    LONG searchFrom = 0;
    for (std::wsregex_iterator it(text.begin(), text.end(), timestamp), end; it != end; ++it)
    {
        const std::wstring token = it->str();
        FINDTEXTEXW find = {};
        find.chrg.cpMin = searchFrom; find.chrg.cpMax = -1; find.lpstrText = token.c_str();
        const LRESULT found = SendMessageW(m_parsed, EM_FINDTEXTEXW, FR_DOWN, reinterpret_cast<LPARAM>(&find));
        if (found < 0) continue;
        SendMessageW(m_parsed, EM_EXSETSEL, 0, reinterpret_cast<LPARAM>(&find.chrgText));
        SendMessageW(m_parsed, EM_SETCHARFORMAT, SCF_SELECTION, reinterpret_cast<LPARAM>(&stamp));
        searchFrom = find.chrgText.cpMax;
    }
}

void BeatDropLyricsEditor::HandleParsedKey(WPARAM key)
{
    if (key == VK_DELETE) ResetSelectedTimestamp();
}

void BeatDropLyricsEditor::ResetSelectedTimestamp()
{
    if (!m_parsed) return;
    PushUndo();
    CHARRANGE selection = {};
    SendMessageW(m_parsed, EM_EXGETSEL, 0, reinterpret_cast<LPARAM>(&selection));
    const LONG targetLine = static_cast<LONG>(SendMessageW(m_parsed, EM_LINEFROMCHAR, selection.cpMin, 0));
    std::wstring document = Read(m_parsed);
    size_t lineStart = 0;
    for (LONG index = 0; index < targetLine && lineStart < document.size(); ++index)
    {
        const size_t newline = document.find(L'\n', lineStart);
        if (newline == std::wstring::npos) { lineStart = document.size(); break; }
        lineStart = newline + 1;
    }
    if (lineStart >= document.size()) return;
    const size_t lineEnd = document.find_first_of(L"\r\n", lineStart);
    const size_t actualEnd = lineEnd == std::wstring::npos ? document.size() : lineEnd;
    std::wstring lineText = document.substr(lineStart, actualEnd - lineStart);
    const std::wregex stamp(LR"(^\[[0-9]+:[0-9]{2}(?:\.[0-9]{1,3})?\])");
    std::wsmatch match;
    if (!std::regex_search(lineText, match, stamp)) return;
    size_t removeLength = match.length();
    if (removeLength < lineText.size() && lineText[removeLength] == L' ') ++removeLength;
    document.erase(lineStart, removeLength);
    Write(m_parsed, document);
    FormatParsedText();
    const LONG updatedStart = static_cast<LONG>(SendMessageW(m_parsed, EM_LINEINDEX, targetLine, 0));
    const LONG updatedLength = updatedStart >= 0 ? static_cast<LONG>(SendMessageW(m_parsed, EM_LINELENGTH, updatedStart, 0)) : 0;
    CHARRANGE updatedSelection{updatedStart, updatedStart + (std::max)(0L, updatedLength)};
    SendMessageW(m_parsed, EM_EXSETSEL, 0, reinterpret_cast<LPARAM>(&updatedSelection));
    SetFocus(m_parsed);
    SetStatus(L"Removed the timestamp from the selected line.");
}
