#pragma once

#include <windows.h>
#include <string>
#include <vector>

class BeatDropLyricsManager;

// Native port of the Lrcdrop lyrics editor surface.  The editor deliberately
// owns the two text panes and editing history; the lyrics manager owns the
// current-song cache and timed display data.
class BeatDropLyricsEditor
{
public:
    void Open(HWND owner, BeatDropLyricsManager* manager,
              const std::wstring& artist, const std::wstring& title);

private:
    static LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);
    static LRESULT CALLBACK ParsedEditProc(HWND, UINT, WPARAM, LPARAM);
    LRESULT HandleMessage(UINT, WPARAM, LPARAM);
    void ResizeControls();
    void ParseText();
    void Save();
    void Upload();
    void CaptureTimestamp();
    void CaptureAndAdvance();
    void InsertSilence();
    void DeleteSelectedLine();
    void ResetSelectedTimestamp();
    void MoveSelectedLine(int direction);
    void Undo();
    void Redo();
    void Reset();
    void SeekToLine();
    void Import();
    void SetStatus(const std::wstring& text);
    std::wstring Read(HWND control) const;
    void Write(HWND control, const std::wstring& text);
    void PushUndo();
    void FormatParsedText();
    void HandleParsedKey(WPARAM key);
    void HandleEditorSelection();

    HWND m_hwnd = nullptr;
    HWND m_input = nullptr;
    HWND m_parsed = nullptr;
    HWND m_status = nullptr;
    HWND m_position = nullptr;
    WNDPROC m_oldParsedProc = nullptr;
    BeatDropLyricsManager* m_manager = nullptr;
    std::wstring m_artist;
    std::wstring m_title;
    std::wstring m_lastLoadedLrc;
    std::vector<std::wstring> m_undo;
    std::vector<std::wstring> m_redo;
    int m_selectedLine = -1;
};
