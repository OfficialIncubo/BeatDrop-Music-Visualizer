#pragma once

#include <windows.h>

#include <string>
#include <vector>

class BeatDropLyricsManager;

// Native timed-LRC editor adapted from Goury's original BeatDrop lyrics work.
// The editor is intentionally separate from the renderer: it can be used to
// import and repair local LRC files even where Now Playing is unavailable.
class BeatDropLyricsEditor
{
public:
    void Open(HWND owner, BeatDropLyricsManager* manager,
        const std::wstring& artist, const std::wstring& title);

private:
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK ParsedEditProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    LRESULT HandleMessage(UINT message, WPARAM wParam, LPARAM lParam);

    void ResizeControls();
    void ParseText();
    void Save();
    void ClearCache();
    void Upload();
    void Import();
    void CaptureTimestamp();
    void CaptureAndAdvance();
    void InsertSilence();
    void DeleteSelectedLine();
    void ResetSelectedTimestamp();
    void MoveSelectedLine(int direction);
    void SeekToLine();
    void Undo();
    void Redo();
    void Reset();
    void PushUndo();
    void UpdateHistoryButtons();
    void FormatParsedText();
    void HandleEditorSelection();
    void SelectLine(int line);
    int SelectedLine() const;
    std::wstring Read(HWND control) const;
    void Write(HWND control, const std::wstring& text);
    void SetStatus(const std::wstring& text);
    bool AutoApplyEnabled() const;
    void ApplyParsedToRenderer(bool force = false);

    HWND m_hwnd = nullptr;
    HWND m_input = nullptr;
    HWND m_parsed = nullptr;
    HWND m_status = nullptr;
    WNDPROC m_oldInputProc = nullptr;
    WNDPROC m_oldParsedProc = nullptr;
    BeatDropLyricsManager* m_manager = nullptr;
    std::wstring m_artist;
    std::wstring m_title;
    std::wstring m_lastLoadedLrc;
    std::wstring m_lastAppliedParsed;
    struct HistoryState
    {
        std::wstring input;
        std::wstring parsed;
    };
    std::vector<HistoryState> m_undo;
    std::vector<HistoryState> m_redo;
    int m_selectedLine = 0;
    bool m_applyingHistory = false;
    bool m_writingText = false;
};
