#pragma once

inline memify mem("cs2.exe");
uintptr_t client = 0;
uintptr_t engine = 0;
HWND hwnd;

float FOV;
HMODULE dll = LoadLibrary("user32.dll");
typedef void (WINAPI* MouseMove)(_In_ DWORD, _In_ DWORD, _In_ DWORD, _In_ DWORD, _In_ ULONG_PTR);
MouseMove mm;

static int lockedTargetIndex = -1;
bool wasKeyHeldLastFrame = false;
Vector previousTargetPosition = { 0.0f, 0.0f, 0.0f };
const float maxAllowedFlickDistance = 50.0f;

namespace offset
{
    constexpr std::ptrdiff_t dwEntityList = 0x1A37A30;
    constexpr std::ptrdiff_t dwViewMatrix = 0x1AA3810;
    constexpr std::ptrdiff_t dwLocalPlayerPawn = 0x188BF30;
    constexpr std::ptrdiff_t m_hPlayerPawn = 0x80C;
    constexpr std::ptrdiff_t m_iHealth = 0x344;
    constexpr std::ptrdiff_t m_iTeamNum = 0x3E3;
    constexpr std::ptrdiff_t m_vOldOrigin = 0x1324;
    constexpr std::ptrdiff_t m_modelState = 0x170;
    constexpr std::ptrdiff_t m_pGameSceneNode = 0x328;
    constexpr std::ptrdiff_t m_pClippingWeapon = 0x13A0;
    constexpr std::ptrdiff_t m_Item = 0x50;
    constexpr std::ptrdiff_t m_iItemDefinitionIndex = 0x1BA;
    constexpr std::ptrdiff_t m_AttributeManager = 0x1148;
    constexpr std::ptrdiff_t dwWindowWidth = 0x623518;
    constexpr std::ptrdiff_t dwWindowHeight = 0x62351C;
    constexpr std::ptrdiff_t dwViewAngles = 0x1AACA70;
    constexpr std::ptrdiff_t m_fFlags = 0x3EC;
    constexpr std::ptrdiff_t dwGlobalVars = 0x187FC90;

    namespace buttons {
        constexpr std::ptrdiff_t attack = 0x1884730;
        constexpr std::ptrdiff_t attack2 = 0x18847C0;
        constexpr std::ptrdiff_t back = 0x1884A00;
        constexpr std::ptrdiff_t duck = 0x1884CD0;
        constexpr std::ptrdiff_t forward = 0x1884970;
        constexpr std::ptrdiff_t jump = 0x1884C40;
        constexpr std::ptrdiff_t left = 0x1884A90;
        constexpr std::ptrdiff_t lookatweapon = 0x1AAD5F0;
        constexpr std::ptrdiff_t reload = 0x18846A0;
        constexpr std::ptrdiff_t right = 0x1884B20;
        constexpr std::ptrdiff_t showscores = 0x1AAD4D0;
        constexpr std::ptrdiff_t sprint = 0x1884610;
        constexpr std::ptrdiff_t turnleft = 0x1884850;
        constexpr std::ptrdiff_t turnright = 0x18848E0;
        constexpr std::ptrdiff_t use = 0x1884BB0;
        constexpr std::ptrdiff_t zoom = 0x1AAD560;
    }
}