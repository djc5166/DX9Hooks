#define DIRECTINPUT_VERSION 0x0800

#include <windows.h>
#include <dinput.h>
#include <tchar.h>
#include <MinHook.h>

constexpr auto MAXCORES = 12;

typedef	LPCDIDATAFORMAT(WINAPI* GetdfDIJoystickProc)    ();
typedef         HRESULT(WINAPI* DirectInput8CreateProc) (HINSTANCE, DWORD, REFIID, LPVOID*, LPUNKNOWN);
typedef         HRESULT(WINAPI* DllCanUnloadNowProc)    ();
typedef         HRESULT(WINAPI* DllGetClassObjectProc)  (REFCLSID, REFIID, LPVOID*);
typedef         HRESULT(WINAPI* DllRegisterServerProc)  ();
typedef         HRESULT(WINAPI* DllUnregisterServerProc)();
typedef           void (WINAPI* GetSystemInfoProc)      (LPSYSTEM_INFO);

GetdfDIJoystickProc     fpGetdfDIJoystick = nullptr;
DirectInput8CreateProc  fpDirectInput8Create = nullptr;
DllCanUnloadNowProc     fpDllCanUnloadNow = nullptr;
DllGetClassObjectProc   fpDllGetClassObject = nullptr;
DllRegisterServerProc   fpDllRegisterServer = nullptr;
DllUnregisterServerProc fpDllUnregisterServer = nullptr;
GetSystemInfoProc       fpGetSystemInfo = nullptr;
HMODULE                 hDinput8dll = nullptr;

void WINAPI hkGetSystemInfo(LPSYSTEM_INFO siSysInfo)
{
    fpGetSystemInfo(siSysInfo);

    if (siSysInfo->dwNumberOfProcessors > MAXCORES)
    {
        siSysInfo->dwNumberOfProcessors = MAXCORES;
        siSysInfo->dwActiveProcessorMask = ((1 << MAXCORES) - 1);
    }
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    {
        if (MH_Initialize() == MH_OK)
        {
            MH_CreateHookApi(TEXT("kernel32"), "GetSystemInfo", &hkGetSystemInfo, reinterpret_cast<LPVOID*>(&fpGetSystemInfo));
            MH_EnableHook(MH_ALL_HOOKS);
        }
        break;
    }
    case DLL_PROCESS_DETACH:
    {
        if (hDinput8dll != nullptr)
            FreeLibrary(hDinput8dll);

        MH_Uninitialize();
        break;
    }
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    default:
    {
        break;
    }
    }
    return TRUE;
}

void InitDI8()
{
    if (hDinput8dll != nullptr)
        return;

    TCHAR sysPath[MAX_PATH];
    GetSystemDirectory(sysPath, MAX_PATH);

    _tcscat_s(sysPath, TEXT("\\dinput8.dll"));
    hDinput8dll = LoadLibrary(sysPath);
}

LPCDIDATAFORMAT WINAPI GetdfDIJoystick()
{
    InitDI8();
    if (fpGetdfDIJoystick == nullptr)
    {
        fpGetdfDIJoystick = reinterpret_cast<GetdfDIJoystickProc>(GetProcAddress(hDinput8dll, "GetdfDIJoystick"));
        if (fpGetdfDIJoystick == nullptr)
            return nullptr;
    }
    return fpGetdfDIJoystick();
}

HRESULT WINAPI DirectInput8Create(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID* ppvOut, LPUNKNOWN punkOuter)
{
    InitDI8();
    if (fpDirectInput8Create == nullptr)
    {
        fpDirectInput8Create = reinterpret_cast<DirectInput8CreateProc>(GetProcAddress(hDinput8dll, "DirectInput8Create"));
        if (fpDirectInput8Create == nullptr)
            return E_FAIL;
    }
    return fpDirectInput8Create(hinst, dwVersion, riidltf, ppvOut, punkOuter);
}

HRESULT WINAPI DllCanUnloadNow()
{
    InitDI8();
    if (fpDllCanUnloadNow == nullptr)
    {
        fpDllCanUnloadNow = reinterpret_cast<DllCanUnloadNowProc>(GetProcAddress(hDinput8dll, "DllCanUnloadNow"));
        if (fpDllCanUnloadNow == nullptr)
            return E_FAIL;
    }
    return fpDllCanUnloadNow();
}

HRESULT WINAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    InitDI8();
    if (fpDllGetClassObject == nullptr)
    {
        fpDllGetClassObject = reinterpret_cast<DllGetClassObjectProc>(GetProcAddress(hDinput8dll, "DllGetClassObject"));
        if (fpDllGetClassObject == nullptr)
            return E_FAIL;
    }
    return fpDllGetClassObject(rclsid, riid, ppv);
}

HRESULT WINAPI DllRegisterServer()
{
    InitDI8();
    if (fpDllRegisterServer == nullptr)
    {
        fpDllRegisterServer = reinterpret_cast<DllRegisterServerProc>(GetProcAddress(hDinput8dll, "DllRegisterServer"));
        if (fpDllRegisterServer == nullptr)
            return E_FAIL;
    }
    return fpDllRegisterServer();
}

HRESULT WINAPI DllUnregisterServer()
{
    InitDI8();
    if (fpDllUnregisterServer == nullptr)
    {
        fpDllUnregisterServer = reinterpret_cast<DllUnregisterServerProc>(GetProcAddress(hDinput8dll, "DllUnregisterServer"));
        if (fpDllUnregisterServer == nullptr)
            return E_FAIL;
    }
    return fpDllUnregisterServer();
}
