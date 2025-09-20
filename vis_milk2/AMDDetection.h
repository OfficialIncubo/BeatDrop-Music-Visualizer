#ifndef _AMDDetection_h_
#define _AMDDetection_h_

#include <iostream>
#include <cstring>

#if defined(_MSC_VER)
#include <intrin.h>
#include <windows.h>
#include <dxgi.h>
#include <comdef.h>
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")
#endif

bool is_amd_cpu();
bool is_amd_gpu();
bool is_amd_ati();

#endif