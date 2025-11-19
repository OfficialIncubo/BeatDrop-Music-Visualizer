#ifndef _MilkDrop2PcmVisualizer_h_
#define _MilkDrop2PcmVisualizer_h_

static bool fullscreen = false;
static bool stretch = false;
static bool borderless = false;

void PrecachePresetShaders(std::wstring& wLine, std::wofstream& compiledList, int& compiledShaders);
void DeleteShaderCacheDirectory();

#endif