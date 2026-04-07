#ifndef _MilkDrop2PcmVisualizer_h_
#define _MilkDrop2PcmVisualizer_h_

extern bool fullscreen;
extern bool stretch;
extern bool borderless;

void ToggleFullScreen(HWND hwnd);
void ToggleStretch(HWND hwnd);
void ToggleBorderlessWindow(HWND hwnd);

void PrecachePresetShaders(std::wstring& wLine, std::wofstream& compiledList, int& compiledShaders);
void DeleteShaderCacheDirectory();

#endif