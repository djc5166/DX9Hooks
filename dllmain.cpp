#define DIRECTINPUT_VERSION 0x0800
#define WIN32_LEAN_AND_MEAN

#include <Windows.h>

#include <dinput.h>
#include <tchar.h>
#include <MinHook.h>

constexpr DWORD MAX_CORES = 12;

//dinput8
typedef LPCDIDATAFORMAT(WINAPI* GetdfDIJoystickProc)(VOID);
typedef HRESULT(WINAPI* DirectInput8CreateProc)(HINSTANCE, DWORD, REFIID, LPVOID*, LPUNKNOWN);
typedef HRESULT(WINAPI* DllCanUnloadNowProc)(VOID);
typedef HRESULT(WINAPI* DllGetClassObjectProc)(REFCLSID, REFIID, LPVOID*);
typedef HRESULT(WINAPI* DllRegisterServerProc)(VOID);
typedef HRESULT(WINAPI* DllUnregisterServerProc)(VOID);

GetdfDIJoystickProc oGetdfDIJoystick = nullptr;
DirectInput8CreateProc oDirectInput8Create = nullptr;
DllCanUnloadNowProc oDllCanUnloadNow = nullptr;
DllGetClassObjectProc oDllGetClassObject = nullptr;
DllRegisterServerProc oDllRegisterServer = nullptr;
DllUnregisterServerProc oDllUnregisterServer = nullptr;

//kernel32
typedef VOID(WINAPI* GetSystemInfoProc)(LPSYSTEM_INFO);

GetSystemInfoProc oGetSystemInfo = nullptr;

//dinput8 handle
HMODULE hDinput8dll = nullptr;

VOID WINAPI hkGetSystemInfo(LPSYSTEM_INFO lpSystemInfo)
{
    oGetSystemInfo(lpSystemInfo);

    if (lpSystemInfo->dwNumberOfProcessors > MAX_CORES)
    {
        lpSystemInfo->dwNumberOfProcessors = MAX_CORES;
        lpSystemInfo->dwActiveProcessorMask = ((1 << MAX_CORES) - 1);
    }
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH:
        {
            if (MH_Initialize() == MH_OK)
            {
                MH_CreateHookApi(L"kernel32", "GetSystemInfo", &hkGetSystemInfo, reinterpret_cast<LPVOID*>(&oGetSystemInfo));
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

VOID InitDI8(VOID)
{
    if (hDinput8dll != nullptr)
        return;

    TCHAR sysPath[MAX_PATH];
    GetSystemDirectory(sysPath, MAX_PATH);

    _tcscat_s(sysPath, TEXT("\\dinput8.dll"));
    hDinput8dll = LoadLibrary(sysPath);
}

LPCDIDATAFORMAT WINAPI GetdfDIJoystick(VOID)
{
    if (oGetdfDIJoystick == nullptr)
    {
        oGetdfDIJoystick = reinterpret_cast<GetdfDIJoystickProc>(GetProcAddress(hDinput8dll, "GetdfDIJoystick"));
        if (oGetdfDIJoystick == nullptr)
            return nullptr;
    }
    return oGetdfDIJoystick();
}

HRESULT WINAPI DirectInput8Create(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID* ppvOut, LPUNKNOWN punkOuter)
{
    InitDI8();
    if (oDirectInput8Create == nullptr)
    {
        oDirectInput8Create = reinterpret_cast<DirectInput8CreateProc>(GetProcAddress(hDinput8dll, "DirectInput8Create"));
        if (oDirectInput8Create == nullptr)
            return E_FAIL;
    }
    return oDirectInput8Create(hinst, dwVersion, riidltf, ppvOut, punkOuter);
}

STDAPI DllCanUnloadNow(VOID)
{
    if (oDllCanUnloadNow == nullptr)
    {
        oDllCanUnloadNow = reinterpret_cast<DllCanUnloadNowProc>(GetProcAddress(hDinput8dll, "DllCanUnloadNow"));
        if (oDllCanUnloadNow == nullptr)
            return E_FAIL;
    }
    return oDllCanUnloadNow();
}

STDAPI DllGetClassObject(_In_ REFCLSID rclsid, _In_ REFIID riid, _Outptr_ LPVOID* ppv)
{
    if (oDllGetClassObject == nullptr)
    {
        oDllGetClassObject = reinterpret_cast<DllGetClassObjectProc>(GetProcAddress(hDinput8dll, "DllGetClassObject"));
        if (oDllGetClassObject == nullptr)
            return E_FAIL;
    }
    return oDllGetClassObject(rclsid, riid, ppv);
}

STDAPI DllRegisterServer(VOID)
{
    if (oDllRegisterServer == nullptr)
    {
        oDllRegisterServer = reinterpret_cast<DllRegisterServerProc>(GetProcAddress(hDinput8dll, "DllRegisterServer"));
        if (oDllRegisterServer == nullptr)
            return E_FAIL;
    }
    return oDllRegisterServer();
}

STDAPI DllUnregisterServer(VOID)
{
    if (oDllUnregisterServer == nullptr)
    {
        oDllUnregisterServer = reinterpret_cast<DllUnregisterServerProc>(GetProcAddress(hDinput8dll, "DllUnregisterServer"));
        if (oDllUnregisterServer == nullptr)
            return E_FAIL;
    }
    return oDllUnregisterServer();
}

