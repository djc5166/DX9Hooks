# DX9Hooks
DINPUT8 DLL for hooking GetSystemInfo() to fix the number of processors reported to the application at 12

## Fixes (AFAIK)
- 12 max logical cores
    - Darksiders (Original)
    - LostPlanet2 (Non-steam Version)

- 26 max logical cores
    - Warhammer 40,000: Space Marine

## Requirements

- [VS2022 Community](https://visualstudio.microsoft.com/vs/community/)
- [vcpkg](https://github.com/microsoft/vcpkg)
    - [minhook v1\.3\.3\#3 \(x86\-windows\-static\)](https://github.com/TsudaKageyu/minhook)
