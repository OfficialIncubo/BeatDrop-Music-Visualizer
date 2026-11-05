#include "hdr.h"
#include <d3d11.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <wrl/client.h>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstring>
#include <vector>
using Microsoft::WRL::ComPtr;
namespace {
struct Module {
    HMODULE handle = nullptr;
    ~Module() { if (handle) FreeLibrary(handle); }
    bool Load(const wchar_t* name) {
        wchar_t dir[MAX_PATH];
        if (!GetSystemDirectoryW(dir, MAX_PATH)) return false;
        std::wstring path = std::wstring(dir) + L"\\" + name;
        handle = LoadLibraryW(path.c_str()); return handle != nullptr;
    }
};
const char* shader = R"(
Texture2D frameTex:register(t0);
cbuffer Levels:register(b0) { float whiteScale; float peakScale; float highlightStrength; float compression; };
struct V { float4 p:SV_POSITION; };
V vs(uint id:SV_VertexID) { V o; o.p=float4(id==2?3:-1,id==1?3:-1,0,1); return o; }
float decodeLegacyColor(float x) {
    if (!isfinite(x)) return 0;
    x=max(x,0);
    if (x<=.04045) return x/12.92;
    if (x<=1) return pow((x+.055)/1.055,2.4);
    // Presets use display-encoded colors, not scene-linear light. Extend the
    // sRGB curve tangentially above white rather than exponentiating arbitrary
    // legacy over-range values into huge highlights. No preset code is changed.
    return 1+(x-1)*(2.4/1.055);
}
float4 ps(V i):SV_TARGET {
    float3 encoded=frameTex.Load(int3(int2(i.p.xy),0)).rgb;
    float3 sceneLinear=float3(decodeLegacyColor(encoded.r),decodeLegacyColor(encoded.g),decodeLegacyColor(encoded.b));
    float level=max(sceneLinear.r,max(sceneLinear.g,sceneLinear.b));
    // Text/UI and all SDR-range colors match Windows reference white. This
    // branch also preserves very dark colors without a division epsilon floor.
    if (level<=1) return float4(sceneLinear*whiteScale,1);
    float room=max(peakScale-whiteScale,0);
    float extra=(level-1)*whiteScale*highlightStrength;
    float mapped=whiteScale;
    if (room>0) mapped+=room*(extra/(room*compression+extra));
    return float4(sceneLinear*(mapped/level),1);
}

)";
// Inverse tone mapping of the completed preset only. Normalized legacy
// colors cannot exceed reference white on their own. Lift bright luminance
// smoothly, preserving linear RGB ratios and leaving shadows/midtones intact.
// This is display enhancement, not recovered scene detail or preset feedback.
const char* presetHighlightShader = R"(
sampler2D image:register(s0);
float4 controls:register(c0);
float decode(float x) {
    if (!isfinite(x)) return 0;
    x=max(x,0);
    return x>1?1+(x-1)*(2.4/1.055):(x<=.04045?x/12.92:pow((x+.055)/1.055,2.4));
}
float encode(float x) {return x>1?1+(x-1)*(1.055/2.4):(x<=.0031308?x*12.92:1.055*pow(x,1/2.4)-.055);}
float4 main(float2 uv:TEXCOORD0):COLOR0 {
    float4 pixel=tex2D(image,uv);
    float3 light=float3(decode(pixel.r),decode(pixel.g),decode(pixel.b));
    float luminance=dot(light,float3(.2126,.7152,.0722));
    float weight=smoothstep(.5,1,luminance);
    light*=1+controls.x*weight/max(1,luminance);
    return float4(encode(light.r),encode(light.g),encode(light.b),pixel.a);
}
)";
// Decode glyph color before applying brightness. Alpha controls coverage and
// animation, so it must never be amplified with RGB.
const char* textShader = R"(
sampler2D glyph:register(s0);
float4 brightness:register(c0);
float decode(float x) {return x<=.04045?x/12.92:pow((x+.055)/1.055,2.4);}
float encode(float x) {return x>1?1+(x-1)*(1.055/2.4):(x<=.0031308?x*12.92:1.055*pow(x,1/2.4)-.055);}
float4 main(float2 uv:TEXCOORD0,float4 color:COLOR0):COLOR0 {
    float4 pixel=tex2D(glyph,uv)*color;
    float3 light=float3(decode(pixel.r),decode(pixel.g),decode(pixel.b))*brightness.x;
    return float4(encode(light.r),encode(light.g),encode(light.b),pixel.a);
}
)";
const char* titleVertexShader=R"(
row_major float4x4 transform:register(c0);
struct V {float4 p:POSITION0;float4 color:COLOR0;float2 uv:TEXCOORD0;};
V main(float3 p:POSITION0,float4 color:COLOR0,float2 uv:TEXCOORD0) {
    V o;o.p=mul(float4(p,1),transform);o.color=color;o.uv=uv;return o;
}
)";
const char* exportShader = R"(sampler2D frameTex:register(s0);
float4 controls:register(c0); // white, peak, highlight strength, compression
float decodeLegacyColor(float x) {
    if (!isfinite(x)) return 0;
    x=max(x,0);
    if (x<=.04045) return x/12.92;
    if (x<=1) return pow((x+.055)/1.055,2.4);
    // Presets use display-encoded colors, not scene-linear light. Extend the
    // sRGB curve tangentially above white rather than exponentiating arbitrary
    // legacy over-range values into huge highlights. No preset code is changed.
    return 1+(x-1)*(2.4/1.055);
}
float4 main(float2 uv:TEXCOORD0):COLOR0 {
    float3 x=tex2D(frameTex,uv).rgb;
    float3 light=float3(decodeLegacyColor(x.r),decodeLegacyColor(x.g),decodeLegacyColor(x.b));
    float level=max(light.r,max(light.g,light.b));
    float room=max(controls.y/controls.x-1,0);
    if(level>1) {
        float extra=(level-1)*controls.z;
        float mapped=1;
        if(room>0) mapped+=room*extra/(room*controls.w+extra);
        light*=mapped/level; level=mapped;
    }
    // Preserve shadows/midtones; roll highlights smoothly into SDR white.
    if(level>.75) {
        float excess=level-.75;
        light*=(.75+.25*excess/(.25+excess))/level;
    }
    float3 encoded;
    encoded.r=light.r<=.0031308?12.92*light.r:1.055*pow(light.r,1/2.4)-.055;
    encoded.g=light.g<=.0031308?12.92*light.g:1.055*pow(light.g,1/2.4)-.055;
    encoded.b=light.b<=.0031308?12.92*light.b:1.055*pow(light.b,1/2.4)-.055;
    return float4(encoded,1);
}
)";
float SdrWhite(HMONITOR monitor) {
    MONITORINFOEXW mi={};mi.cbSize=sizeof(mi);
    if(!GetMonitorInfoW(monitor,&mi))return 203.0f;
    UINT pathCount=0,modeCount=0;
    if(GetDisplayConfigBufferSizes(QDC_ONLY_ACTIVE_PATHS,&pathCount,&modeCount)!=ERROR_SUCCESS)return 203.0f;
    std::vector<DISPLAYCONFIG_PATH_INFO> paths(pathCount);
    std::vector<DISPLAYCONFIG_MODE_INFO> modes(modeCount);
    if(QueryDisplayConfig(QDC_ONLY_ACTIVE_PATHS,&pathCount,paths.data(),&modeCount,modes.data(),nullptr)!=ERROR_SUCCESS)return 203.0f;
    for(UINT i=0;i<pathCount;++i){
        DISPLAYCONFIG_SOURCE_DEVICE_NAME name={};name.header.type=DISPLAYCONFIG_DEVICE_INFO_GET_SOURCE_NAME;name.header.size=sizeof(name);name.header.adapterId=paths[i].sourceInfo.adapterId;name.header.id=paths[i].sourceInfo.id;
        if(DisplayConfigGetDeviceInfo(&name.header)!=ERROR_SUCCESS||wcscmp(name.viewGdiDeviceName,mi.szDevice))continue;
        DISPLAYCONFIG_SDR_WHITE_LEVEL white={};white.header.type=DISPLAYCONFIG_DEVICE_INFO_GET_SDR_WHITE_LEVEL;white.header.size=sizeof(white);white.header.adapterId=paths[i].targetInfo.adapterId;white.header.id=paths[i].targetInfo.id;
        if(DisplayConfigGetDeviceInfo(&white.header)==ERROR_SUCCESS&&white.SDRWhiteLevel)return white.SDRWhiteLevel*80.0f/1000.0f;
    }
    return 203.0f;
}
bool Wait9(IDirect3DQuery9* query) {
    if (!query || FAILED(query->Issue(D3DISSUE_END))) return false;
    const auto start=std::chrono::steady_clock::now();
    for (;;) {
        const HRESULT hr=query->GetData(nullptr,0,D3DGETDATA_FLUSH);
        if (hr==S_OK) return true;
        if (FAILED(hr) || std::chrono::steady_clock::now()-start>std::chrono::milliseconds(250)) return false;
        SwitchToThread();
    }
}
UINT HdrSyncInterval(UINT interval) {
    switch(interval) {
    case D3DPRESENT_INTERVAL_IMMEDIATE: return 0;
    case D3DPRESENT_INTERVAL_TWO: return 2;
    case D3DPRESENT_INTERVAL_THREE: return 3;
    case D3DPRESENT_INTERVAL_FOUR: return 4;
    default: return 1; // D3D9 DEFAULT/ONE synchronize to vertical blank.
    }
}
bool CreateCopyShader9(IDirect3DDevice9* device, IDirect3DPixelShader9** shaderOut) {
    static const std::vector<DWORD> copyCode = []() {
        std::vector<DWORD> result;
        Module compiler; if(!compiler.Load(L"d3dcompiler_47.dll"))return result;
        auto compile=reinterpret_cast<decltype(&D3DCompile)>(GetProcAddress(compiler.handle,"D3DCompile"));
        const char* code="sampler2D s:register(s0);float4 main(float2 uv:TEXCOORD0):COLOR0{return tex2D(s,uv);}";
        ComPtr<ID3DBlob> blob,error;
        if(compile&&SUCCEEDED(compile(code,strlen(code),nullptr,nullptr,nullptr,"main","ps_3_0",D3DCOMPILE_OPTIMIZATION_LEVEL3,0,&blob,&error))) {
            auto* words=static_cast<DWORD*>(blob->GetBufferPointer());
            result.assign(words,words+blob->GetBufferSize()/sizeof(DWORD));
        }
        return result;
    }();
    return device && !copyCode.empty() && SUCCEEDED(device->CreatePixelShader(copyCode.data(),shaderOut));
}
}
bool CopyHdrTexture9(IDirect3DDevice9* source,IDirect3DTexture9* texture,IDirect3DSurface9* destination,IDirect3DPixelShader9* copyShader,bool sceneOpen){
    ComPtr<IDirect3DSurface9> original;
    ComPtr<IDirect3DStateBlock9> state;
    ComPtr<IDirect3DPixelShader9> temporaryShader;
    D3DSURFACE_DESC desc={};
    if(!source||!texture||!destination||FAILED(destination->GetDesc(&desc))||FAILED(source->GetRenderTarget(0,&original))||FAILED(source->CreateStateBlock(D3DSBT_ALL,&state))||FAILED(state->Capture()))return false;
    if (!copyShader) {
        if (!CreateCopyShader9(source,&temporaryShader)) return false;
        copyShader=temporaryShader.Get();
    }
    if(FAILED(source->SetRenderTarget(0,destination)))return false;
    if(!sceneOpen && FAILED(source->BeginScene())){source->SetRenderTarget(0,original.Get());state->Apply();return false;}
    struct Vertex{float x,y,z,w,u,v;};
    float w=static_cast<float>(desc.Width),h=static_cast<float>(desc.Height);
    Vertex quad[]={{-.5f,-.5f,0,1,0,0},{w-.5f,-.5f,0,1,1,0},{-.5f,h-.5f,0,1,0,1},{w-.5f,h-.5f,0,1,1,1}};
    D3DVIEWPORT9 vp={0,0,desc.Width,desc.Height,0,1};source->SetViewport(&vp);
    // Fixed-function texture stages clamp to [0,1], even with an FP16 target.
    source->SetVertexShader(nullptr);source->SetPixelShader(copyShader);source->SetFVF(D3DFVF_XYZRHW|D3DFVF_TEX1);
    source->SetTexture(0,texture);
    source->SetTextureStageState(0,D3DTSS_COLOROP,D3DTOP_SELECTARG1);source->SetTextureStageState(0,D3DTSS_COLORARG1,D3DTA_TEXTURE);
    source->SetTextureStageState(0,D3DTSS_ALPHAOP,D3DTOP_SELECTARG1);source->SetTextureStageState(0,D3DTSS_ALPHAARG1,D3DTA_TEXTURE);
    source->SetTextureStageState(0,D3DTSS_TEXCOORDINDEX,0);source->SetTextureStageState(0,D3DTSS_TEXTURETRANSFORMFLAGS,D3DTTFF_DISABLE);
    source->SetTextureStageState(1,D3DTSS_COLOROP,D3DTOP_DISABLE);
    source->SetSamplerState(0,D3DSAMP_SRGBTEXTURE,FALSE);source->SetSamplerState(0,D3DSAMP_MINFILTER,D3DTEXF_POINT);source->SetSamplerState(0,D3DSAMP_MAGFILTER,D3DTEXF_POINT);source->SetSamplerState(0,D3DSAMP_MIPFILTER,D3DTEXF_NONE);source->SetSamplerState(0,D3DSAMP_ADDRESSU,D3DTADDRESS_CLAMP);source->SetSamplerState(0,D3DSAMP_ADDRESSV,D3DTADDRESS_CLAMP);
    source->SetRenderState(D3DRS_ALPHABLENDENABLE,FALSE);source->SetRenderState(D3DRS_ALPHATESTENABLE,FALSE);source->SetRenderState(D3DRS_ZENABLE,FALSE);source->SetRenderState(D3DRS_CULLMODE,D3DCULL_NONE);source->SetRenderState(D3DRS_FILLMODE,D3DFILL_SOLID);source->SetRenderState(D3DRS_FOGENABLE,FALSE);source->SetRenderState(D3DRS_STENCILENABLE,FALSE);source->SetRenderState(D3DRS_SCISSORTESTENABLE,FALSE);source->SetRenderState(D3DRS_SRGBWRITEENABLE,FALSE);source->SetRenderState(D3DRS_COLORWRITEENABLE,15);
    const bool ok=SUCCEEDED(source->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP,2,quad,sizeof(Vertex)));
    if(!sceneOpen) source->EndScene();source->SetTexture(0,nullptr);source->SetRenderTarget(0,original.Get());state->Apply();return ok;
}
// Separate display intermediate: never used as preset history or a blur input.
struct LegacyComposite9 {
    ComPtr<IDirect3DTexture9> texture;
    ComPtr<IDirect3DSurface9> display;
    bool Begin(IDirect3DDevice9* source,IDirect3DSurface9* destination,D3DFORMAT format,IDirect3DPixelShader9* copy) {
        if(display)return false;
        ComPtr<IDirect3DSurface9> current,target;
        if(FAILED(source->GetRenderTarget(0,&current))||current.Get()!=destination)return false;
        if(!texture) {
            D3DSURFACE_DESC desc={};
            if(FAILED(destination->GetDesc(&desc))||FAILED(source->CreateTexture(desc.Width,desc.Height,1,D3DUSAGE_RENDERTARGET,format,D3DPOOL_DEFAULT,&texture,nullptr)))return false;
        }
        ComPtr<IDirect3DTexture9> input;
        if(FAILED(destination->GetContainer(__uuidof(IDirect3DTexture9),reinterpret_cast<void**>(input.GetAddressOf())))||FAILED(texture->GetSurfaceLevel(0,&target)))return false;
        if(!CopyHdrTexture9(source,input.Get(),target.Get(),copy,true)||FAILED(source->SetRenderTarget(0,target.Get())))return false;
        display=current;return true;
    }
    bool End(IDirect3DDevice9* source,IDirect3DPixelShader9* copy) {
        if(!display)return false;
        bool ok=CopyHdrTexture9(source,texture.Get(),display.Get(),copy,true);
        source->SetRenderTarget(0,display.Get());display.Reset();return ok;
    }
};
struct HdrPresenter::Impl {
    // Modules outlive all COM objects (members are destroyed in reverse order).
    Module d3d, dxgi, compiler;
    ComPtr<ID3D11Device> device;
    ComPtr<ID3D11DeviceContext> context;
    ComPtr<IDXGIFactory2> factory;
    ComPtr<IDXGIOutput6> output;
    ComPtr<IDXGISwapChain3> chain;
    ComPtr<ID3D11RenderTargetView> target;
    ComPtr<ID3D11VertexShader> vs;
    ComPtr<ID3D11PixelShader> ps;
    ComPtr<ID3D11Buffer> levels;
    ComPtr<IDirect3DTexture9> shared9;
    ComPtr<IDirect3DTexture9> cleanSdr;
    ComPtr<IDirect3DTexture9> presetDisplay;
    LegacyComposite9 legacy;
    bool exportReady=false;
    ComPtr<ID3D11ShaderResourceView> shared11;
    ComPtr<IDirect3DDevice9> sourceDevice;
    ComPtr<IDirect3DPixelShader9> copyShader;
    ComPtr<IDirect3DPixelShader9> sdrExport;
    ComPtr<IDirect3DPixelShader9> presetHighlights;
    ComPtr<IDirect3DPixelShader9> textPixelShader;
    ComPtr<IDirect3DVertexShader9> titleVertex;
    ComPtr<IDirect3DQuery9> ready9;
    ComPtr<ID3D11Query> done11;
    DXGI_OUTPUT_DESC1 displayInfo={};
    float whiteNits=80;
    std::chrono::steady_clock::time_point nextWhiteRead;
    HWND child=nullptr, parent=nullptr;
    UINT width=0,height=0;
    UINT syncInterval=1;
    bool frameReady=false;
    ComPtr<IDirect3DSurface9> originalTarget;
    ~Impl() { if(originalTarget&&sourceDevice) sourceDevice->SetRenderTarget(0,originalTarget.Get()); if(context) {context->ClearState();context->Flush();} if(child && IsWindow(child)) DestroyWindow(child); }
    bool Initialize(IDirect3DDevice9* source,HWND window) {
        sourceDevice=source; parent=window;
        ComPtr<IDirect3DSurface9> back;
        D3DSURFACE_DESC desc={};
        if(FAILED(source->GetBackBuffer(0,0,D3DBACKBUFFER_TYPE_MONO,&back))||FAILED(back->GetDesc(&desc)))return false;
        width=desc.Width;height=desc.Height;
        ComPtr<IDirect3DSwapChain9> swap9;D3DPRESENT_PARAMETERS present9={};
        if(FAILED(source->GetSwapChain(0,&swap9))||FAILED(swap9->GetPresentParameters(&present9))||!present9.Windowed)return false;
        if(!d3d.Load(L"d3d11.dll")||!dxgi.Load(L"dxgi.dll")||!compiler.Load(L"d3dcompiler_47.dll"))return false;
        auto createFactory=reinterpret_cast<HRESULT(WINAPI*)(REFIID,void**)>(GetProcAddress(dxgi.handle,"CreateDXGIFactory1"));
        auto createDevice=reinterpret_cast<decltype(&D3D11CreateDevice)>(GetProcAddress(d3d.handle,"D3D11CreateDevice"));
        auto compile=reinterpret_cast<decltype(&D3DCompile)>(GetProcAddress(compiler.handle,"D3DCompile"));
        if(!createFactory||!createDevice||!compile)return false;
        if(FAILED(createFactory(__uuidof(IDXGIFactory2),reinterpret_cast<void**>(factory.GetAddressOf()))))return false;
        ComPtr<IDirect3D9> d9;ComPtr<IDirect3D9Ex> d9ex;D3DDEVICE_CREATION_PARAMETERS params={}; LUID luid={};
        if(FAILED(source->GetDirect3D(&d9))||FAILED(d9.As(&d9ex))||FAILED(source->GetCreationParameters(&params))||FAILED(d9ex->GetAdapterLUID(params.AdapterOrdinal,&luid)))return false;
        D3DDISPLAYMODE mode={};
        if (FAILED(d9->GetAdapterDisplayMode(params.AdapterOrdinal,&mode)) ||
            FAILED(d9->CheckDeviceFormat(params.AdapterOrdinal,params.DeviceType,mode.Format,
                D3DUSAGE_RENDERTARGET|D3DUSAGE_QUERY_POSTPIXELSHADER_BLENDING,
                D3DRTYPE_TEXTURE,D3DFMT_A16B16G16R16F))) return false;
        ComPtr<IDXGIAdapter1> adapter;
        for(UINT i=0;;++i){
            ComPtr<IDXGIAdapter1> candidate; if(factory->EnumAdapters1(i,&candidate)!=S_OK)break;
            DXGI_ADAPTER_DESC1 a={};candidate->GetDesc1(&a);
            if(a.AdapterLuid.HighPart==luid.HighPart&&a.AdapterLuid.LowPart==luid.LowPart){adapter=candidate;break;}
        }
        if(!adapter)return false;
        for(UINT i=0;;++i){
            ComPtr<IDXGIOutput> candidate;if(adapter->EnumOutputs(i,&candidate)!=S_OK)break;
            DXGI_OUTPUT_DESC descOut={};candidate->GetDesc(&descOut);
            if(descOut.Monitor==MonitorFromWindow(window,MONITOR_DEFAULTTONEAREST)){candidate.As(&output);break;}
        }
        auto& info=displayInfo;
        if(!output||FAILED(output->GetDesc1(&info))||info.ColorSpace!=DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020)return false;
        D3D_FEATURE_LEVEL feature=D3D_FEATURE_LEVEL_11_0;
        if(FAILED(createDevice(adapter.Get(),D3D_DRIVER_TYPE_UNKNOWN,nullptr,D3D11_CREATE_DEVICE_BGRA_SUPPORT,&feature,1,D3D11_SDK_VERSION,&device,nullptr,&context)))return false;
        D3D11_QUERY_DESC queryDesc={D3D11_QUERY_EVENT,0};
        if (FAILED(source->CreateQuery(D3DQUERYTYPE_EVENT,&ready9)) ||
            FAILED(device->CreateQuery(&queryDesc,&done11)) ||
            !CreateCopyShader9(source,&copyShader)) return false;
        ComPtr<ID3DBlob> textCode,textError;
        if (FAILED(compile(textShader,strlen(textShader),nullptr,nullptr,nullptr,"main","ps_3_0",D3DCOMPILE_OPTIMIZATION_LEVEL3,0,&textCode,&textError)) ||
            FAILED(source->CreatePixelShader(static_cast<DWORD*>(textCode->GetBufferPointer()),&textPixelShader))) return false;
        ComPtr<ID3DBlob> presetCode;
        if (FAILED(compile(presetHighlightShader,strlen(presetHighlightShader),nullptr,nullptr,nullptr,"main","ps_3_0",D3DCOMPILE_OPTIMIZATION_LEVEL3,0,&presetCode,&textError)) ||
            FAILED(source->CreatePixelShader(static_cast<DWORD*>(presetCode->GetBufferPointer()),&presetHighlights))) return false;
        ComPtr<ID3DBlob> titleCode;
        if (FAILED(compile(titleVertexShader,strlen(titleVertexShader),nullptr,nullptr,nullptr,"main","vs_3_0",D3DCOMPILE_OPTIMIZATION_LEVEL3,0,&titleCode,&textError)) ||
            FAILED(source->CreateVertexShader(static_cast<DWORD*>(titleCode->GetBufferPointer()),&titleVertex))) return false;
        ComPtr<ID3DBlob> exportCode;
        if (FAILED(compile(exportShader,strlen(exportShader),nullptr,nullptr,nullptr,"main","ps_3_0",D3DCOMPILE_OPTIMIZATION_LEVEL3,0,&exportCode,&textError)) ||
            FAILED(source->CreatePixelShader(static_cast<DWORD*>(exportCode->GetBufferPointer()),&sdrExport))) return false;
        HANDLE shared=nullptr;
        if(FAILED(source->CreateTexture(width,height,1,D3DUSAGE_RENDERTARGET,D3DFMT_A16B16G16R16F,D3DPOOL_DEFAULT,&shared9,&shared)))return false;
        if(FAILED(source->CreateTexture(width,height,1,D3DUSAGE_RENDERTARGET,D3DFMT_A8R8G8B8,D3DPOOL_DEFAULT,&cleanSdr,nullptr)))return false;
        ComPtr<ID3D11Texture2D> texture;
        if(FAILED(device->OpenSharedResource(shared,__uuidof(ID3D11Texture2D),reinterpret_cast<void**>(texture.GetAddressOf())))||FAILED(device->CreateShaderResourceView(texture.Get(),nullptr,&shared11)))return false;
        ComPtr<ID3DBlob> v,pixel,error;
        if(FAILED(compile(shader,strlen(shader),nullptr,nullptr,nullptr,"vs","vs_5_0",D3DCOMPILE_OPTIMIZATION_LEVEL3,0,&v,&error))||FAILED(compile(shader,strlen(shader),nullptr,nullptr,nullptr,"ps","ps_5_0",D3DCOMPILE_OPTIMIZATION_LEVEL3,0,&pixel,&error)))return false;
        if(FAILED(device->CreateVertexShader(v->GetBufferPointer(),v->GetBufferSize(),nullptr,&vs))||FAILED(device->CreatePixelShader(pixel->GetBufferPointer(),pixel->GetBufferSize(),nullptr,&ps)))return false;
        D3D11_BUFFER_DESC cb={};cb.ByteWidth=16;cb.Usage=D3D11_USAGE_DEFAULT;cb.BindFlags=D3D11_BIND_CONSTANT_BUFFER;
        if(FAILED(device->CreateBuffer(&cb,nullptr,&levels)))return false;
        child=CreateWindowExW(0,L"STATIC",L"BeatDrop HDR",WS_CHILD|WS_DISABLED,0,0,width,height,window,nullptr,GetModuleHandleW(nullptr),nullptr);
        if(!child)return false;
        DXGI_SWAP_CHAIN_DESC1 swap={};swap.Width=width;swap.Height=height;swap.Format=DXGI_FORMAT_R16G16B16A16_FLOAT;swap.SampleDesc.Count=1;swap.BufferUsage=DXGI_USAGE_RENDER_TARGET_OUTPUT;swap.BufferCount=2;swap.SwapEffect=DXGI_SWAP_EFFECT_FLIP_DISCARD;swap.AlphaMode=DXGI_ALPHA_MODE_IGNORE;
        ComPtr<IDXGISwapChain1> initial;
        if(FAILED(factory->CreateSwapChainForHwnd(device.Get(),child,&swap,nullptr,nullptr,&initial))||FAILED(initial.As(&chain)))return false;
        factory->MakeWindowAssociation(window,DXGI_MWA_NO_ALT_ENTER);
        UINT support=0;
        if(FAILED(chain->CheckColorSpaceSupport(DXGI_COLOR_SPACE_RGB_FULL_G10_NONE_P709,&support))||!(support&DXGI_SWAP_CHAIN_COLOR_SPACE_SUPPORT_FLAG_PRESENT)||FAILED(chain->SetColorSpace1(DXGI_COLOR_SPACE_RGB_FULL_G10_NONE_P709)))return false;
        ComPtr<ID3D11Texture2D> buffer;
        return SUCCEEDED(chain->GetBuffer(0,__uuidof(ID3D11Texture2D),reinterpret_cast<void**>(buffer.GetAddressOf())))&&SUCCEEDED(device->CreateRenderTargetView(buffer.Get(),nullptr,&target));
    }
};
HdrPresenter::HdrPresenter()=default;
HdrPresenter::~HdrPresenter()=default;
void HdrPresenter::Reset(){p.reset();}
bool HdrPresenter::Active()const{return p&&p->chain;}
bool HdrPresenter::Prepare(IDirect3DDevice9* source,HWND window,std::wstring& reason){
    reason.clear();
    ComPtr<IDirect3DSurface9> back;D3DSURFACE_DESC desc={};
    if(!source||!IsWindow(window)||FAILED(source->GetBackBuffer(0,0,D3DBACKBUFFER_TYPE_MONO,&back))||FAILED(back->GetDesc(&desc))){reason=L"HDR source is unavailable. SDR output restored.";Reset();return false;}
    if(p&&(p->sourceDevice.Get()!=source||p->parent!=window||p->width!=desc.Width||p->height!=desc.Height))Reset();
    if(!p){auto next=std::make_unique<Impl>();if(!next->Initialize(source,window)){reason=L"HDR unavailable. Enable Windows HDR on a compatible display; SDR output retained.";return false;}p=std::move(next);}
    // DXGI outputs can become stale after display configuration changes.
    // Do not query a stale output or keep using its old luminance metadata.
    if(!p->factory->IsCurrent()||p->displayInfo.Monitor!=MonitorFromWindow(window,MONITOR_DEFAULTTONEAREST)){reason=L"HDR display changed or HDR is disabled. SDR output restored.";Reset();return false;}
    // Follow the actual player's presentation policy, including changes made
    // by a D3D9 reset that keeps the same device pointer and buffer dimensions.
    ComPtr<IDirect3DSwapChain9> sourceSwap;
    D3DPRESENT_PARAMETERS sourceParameters={};
    if(FAILED(source->GetSwapChain(0,&sourceSwap))||FAILED(sourceSwap->GetPresentParameters(&sourceParameters))){reason=L"HDR frame timing unavailable; SDR output restored.";Reset();return false;}
    p->syncInterval=HdrSyncInterval(sourceParameters.PresentationInterval);
    p->frameReady=false;return true;
}
bool HdrPresenter::BeginFrame(IDirect3DDevice9* source){
    if(!p||source!=p->sourceDevice.Get()||p->originalTarget)return false;
    auto now=std::chrono::steady_clock::now();
    if (now>=p->nextWhiteRead) {
        p->whiteNits=SdrWhite(p->displayInfo.Monitor);
        p->nextWhiteRead=now+std::chrono::seconds(1);
    }
    ComPtr<IDirect3DSurface9> target;
    if(FAILED(source->GetRenderTarget(0,&p->originalTarget))||FAILED(p->shared9->GetSurfaceLevel(0,&target)))return false;
    if(FAILED(source->SetRenderTarget(0,target.Get())))return false;
    p->frameReady=SUCCEEDED(source->Clear(0,nullptr,D3DCLEAR_TARGET,0,1,0));
    return p->frameReady;
}
bool HdrPresenter::BindTextShader(IDirect3DDevice9* source,bool brighten,bool screenSpaceVertices) {
    if (!p || !p->originalTarget || source!=p->sourceDevice.Get()) return false;
    ComPtr<IDirect3DSurface9> current,display;
    if (FAILED(source->GetRenderTarget(0,&current)) || FAILED(p->shared9->GetSurfaceLevel(0,&display)) || current.Get()!=display.Get()) return false;
    float requested=screenSpaceVertices?settings.lyricsBrightness:settings.titleBrightness;
    float gain=brighten?requested:1.0f;
    const float values[]={gain,0,0,0};
    if (!screenSpaceVertices) {
        // Titles use XYZ geometry, whereas lyrics bypass vertex processing with
        // XYZRHW. Supply color/UV semantics explicitly instead of depending on
        // the fixed-function vertex converter and inherited texture routing.
        D3DMATRIX world,view,projection;
        if (FAILED(source->GetTransform(D3DTS_WORLD,&world)) || FAILED(source->GetTransform(D3DTS_VIEW,&view)) || FAILED(source->GetTransform(D3DTS_PROJECTION,&projection))) return false;
        auto multiply=[](const D3DMATRIX& a,const D3DMATRIX& b) {
            D3DMATRIX out={};
            for(int row=0;row<4;++row)for(int col=0;col<4;++col)for(int k=0;k<4;++k)out.m[row][col]+=a.m[row][k]*b.m[k][col];
            return out;
        };
        D3DMATRIX transform=multiply(multiply(world,view),projection);
        if (FAILED(source->SetVertexShader(p->titleVertex.Get())) || FAILED(source->SetVertexShaderConstantF(0,&transform._11,4))) return false;
    }
    return SUCCEEDED(source->SetPixelShader(p->textPixelShader.Get())) &&
        SUCCEEDED(source->SetPixelShaderConstantF(0,values,1));
}
bool HdrPresenter::BeginLegacyComposite(IDirect3DDevice9* source) {
    if(!p||source!=p->sourceDevice.Get()||!p->originalTarget)return false;
    ComPtr<IDirect3DSurface9> display;D3DSURFACE_DESC desc={};
    if(FAILED(p->shared9->GetSurfaceLevel(0,&display))||FAILED(p->originalTarget->GetDesc(&desc)))return false;
    return p->legacy.Begin(source,display.Get(),desc.Format,p->copyShader.Get());
}
bool HdrPresenter::EndLegacyComposite(IDirect3DDevice9* source,bool expandHighlights) {
    if(!p||source!=p->sourceDevice.Get()||!p->originalTarget)return false;
    const float white=settings.paperWhite>0?settings.paperWhite:p->whiteNits;
    const float peak=settings.peak>0?settings.peak:p->displayInfo.MaxLuminance;
    const float values[]={expandHighlights && peak>white?settings.highlights:0,0,0,0};
    float saved[4];source->GetPixelShaderConstantF(0,saved,1);
    source->SetPixelShaderConstantF(0,values,1);
    // Fuse highlight expansion with the existing legacy-to-FP16 transfer.
    bool ok=p->legacy.End(source,values[0]>0?p->presetHighlights.Get():p->copyShader.Get());
    source->SetPixelShaderConstantF(0,saved,1);return ok;
}
bool HdrPresenter::ExpandPresetHighlights(IDirect3DDevice9* source) {
    if(!p||source!=p->sourceDevice.Get()||!p->originalTarget)return false;
    ComPtr<IDirect3DSurface9> display,current;
    if(FAILED(p->shared9->GetSurfaceLevel(0,&display))||FAILED(source->GetRenderTarget(0,&current))||current.Get()!=display.Get())return false;
    const float white=settings.paperWhite>0?settings.paperWhite:p->whiteNits;
    const float peak=settings.peak>0?settings.peak:p->displayInfo.MaxLuminance;
    if(settings.highlights<=0||!(peak>white))return true;
    if(!p->presetDisplay && FAILED(source->CreateTexture(p->width,p->height,1,D3DUSAGE_RENDERTARGET,D3DFMT_A16B16G16R16F,D3DPOOL_DEFAULT,&p->presetDisplay,nullptr)))return false;
    ComPtr<IDirect3DSurface9> temporary;
    if(FAILED(p->presetDisplay->GetSurfaceLevel(0,&temporary))||!CopyHdrTexture9(source,p->shared9.Get(),temporary.Get(),p->copyShader.Get(),true))return false;
    const float values[]={settings.highlights,0,0,0};
    float saved[4];source->GetPixelShaderConstantF(0,saved,1);
    source->SetPixelShaderConstantF(0,values,1);
    bool ok=CopyHdrTexture9(source,p->presetDisplay.Get(),display.Get(),p->presetHighlights.Get(),true);
    source->SetPixelShaderConstantF(0,saved,1);return ok;
}
bool HdrPresenter::CaptureSdrExport(IDirect3DDevice9* source) {
    if(!p||!p->originalTarget||source!=p->sourceDevice.Get())return false;
    ComPtr<IDirect3DSurface9> current,display,destination;
    if(FAILED(source->GetRenderTarget(0,&current))||FAILED(p->shared9->GetSurfaceLevel(0,&display))||current.Get()!=display.Get()||FAILED(p->cleanSdr->GetSurfaceLevel(0,&destination)))return false;
    const float white=settings.paperWhite>0?settings.paperWhite:p->whiteNits;
    float report=std::isfinite(p->displayInfo.MaxLuminance)?p->displayInfo.MaxLuminance:white;
    float values[]={white,(std::max)(white,settings.peak>0?settings.peak:report),settings.highlights,settings.compression},saved[4];
    source->GetPixelShaderConstantF(0,saved,1);source->SetPixelShaderConstantF(0,values,1);
    p->exportReady=CopyHdrTexture9(source,p->shared9.Get(),destination.Get(),p->sdrExport.Get(),true);
    source->SetPixelShaderConstantF(0,saved,1);return p->exportReady;
}
bool HdrPresenter::GetSdrExport(IDirect3DSurface9** surface) const {
    return surface && p && p->exportReady && SUCCEEDED(p->cleanSdr->GetSurfaceLevel(0,surface));
}
bool HdrPresenter::EndFrame(IDirect3DDevice9* source){
    if(!p||source!=p->sourceDevice.Get()||!p->originalTarget)return false;
    const float white=settings.paperWhite>0?settings.paperWhite:p->whiteNits;
    const float report=std::isfinite(p->displayInfo.MaxLuminance)?p->displayInfo.MaxLuminance:white;
    const float peak=(std::max)(white,settings.peak>0?settings.peak:report);
    const float values[]={white,peak,settings.highlights,settings.compression};
    // Copy helper captures/restores state; constants must be isolated too.
    float saved[4];source->GetPixelShaderConstantF(0,saved,1);
    source->SetPixelShaderConstantF(0,values,1);
    const bool ok=CopyHdrTexture9(source,p->shared9.Get(),p->originalTarget.Get(),p->sdrExport.Get());
    source->SetPixelShaderConstantF(0,saved,1);
    source->SetRenderTarget(0,p->originalTarget.Get());
    p->originalTarget.Reset();p->frameReady=ok;return ok;
}

bool HdrPresenter::Present(IDirect3DDevice9* source,std::wstring& reason){
    if(!p||source!=p->sourceDevice.Get()||!p->frameReady||!Wait9(p->ready9.Get())){reason=L"HDR frame transfer failed; SDR output restored.";Reset();return false;}
    const float white=settings.paperWhite>0?settings.paperWhite:p->whiteNits;
    const float reportedPeak=p->displayInfo.MaxLuminance;
    const float peak=(std::max)(white,settings.peak>0?settings.peak:(std::isfinite(reportedPeak)?reportedPeak:white));
    const float values[]={white/80.0f,peak/80.0f,settings.highlights,settings.compression};
    p->context->UpdateSubresource(p->levels.Get(),0,nullptr,values,0,0);
    auto* rt=p->target.Get();p->context->OMSetRenderTargets(1,&rt,nullptr);
    D3D11_VIEWPORT view={0,0,static_cast<float>(p->width),static_cast<float>(p->height),0,1};p->context->RSSetViewports(1,&view);
    auto* cb=p->levels.Get();p->context->PSSetConstantBuffers(0,1,&cb);
    ID3D11ShaderResourceView* srvs[]={p->shared11.Get()};p->context->PSSetShaderResources(0,1,srvs);
    p->context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);p->context->VSSetShader(p->vs.Get(),nullptr,0);p->context->PSSetShader(p->ps.Get(),nullptr,0);p->context->Draw(3,0);
    ID3D11ShaderResourceView* empty=nullptr;p->context->PSSetShaderResources(0,1,&empty);
    const HRESULT hr=p->chain->Present(p->syncInterval,0);
    // Fence the read before D3D9 writes the shared inputs on the next frame.
    if(FAILED(hr)){reason=L"HDR presentation failed; SDR output restored.";Reset();return false;}
    p->context->End(p->done11.Get());p->context->Flush();
    const auto start=std::chrono::steady_clock::now();
    for(;;){const HRESULT done=p->context->GetData(p->done11.Get(),nullptr,0,0);if(done==S_OK)break;if(FAILED(done)||std::chrono::steady_clock::now()-start>std::chrono::milliseconds(250)){reason=L"HDR GPU synchronization failed; SDR output restored.";Reset();return false;}SwitchToThread();}
    RECT client={};GetClientRect(p->parent,&client);SetWindowPos(p->child,HWND_TOP,0,0,client.right,client.bottom,SWP_NOACTIVATE|SWP_SHOWWINDOW);
    return true;
}