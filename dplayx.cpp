#include <iostream>
#include <windows.h>
#include "dplay.h"
#include "dplobby.h"
__declspec(dllexport) auto DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) -> BOOL
{
    if (fdwReason == DLL_PROCESS_ATTACH)
    {
        AllocConsole();

        FILE* f;
        freopen_s(&f, "CONOUT$", "w", stdout);
        printf("dplayx.dll loaded!\n");
    }
    return 1;
}

static HRESULT (WINAPI* oldDirectPlayCreate)(LPGUID, LPDIRECTPLAY *, IUnknown*) = NULL;

static HRESULT (WINAPI* oldDirectPlayEnumerateA)(LPDPENUMDPCALLBACKA, LPVOID) = NULL;
static HRESULT (WINAPI* oldDirectPlayEnumerateW)(LPDPENUMDPCALLBACKW, LPVOID) = NULL;

static HRESULT (WINAPI* oldDirectPlayLobbyCreateA)(LPGUID, LPDIRECTPLAYLOBBY*,  IUnknown*, LPVOID, DWORD) = NULL;
static HRESULT (WINAPI* oldDirectPlayLobbyCreateW)(LPGUID, LPDIRECTPLAYLOBBY*,  IUnknown*, LPVOID, DWORD) = NULL;

extern "C" __declspec(dllexport) DWORD gdwDPlaySPRefCount = 0; // increments by 1 every timne you create or serach for a lobby 

HRESULT WINAPI DirectPlayCreate(LPGUID lpGUID, LPDIRECTPLAY *lplpDP, IUnknown *pUnkOuter)
{
    printf("DirectPlayCreate!\n");
    printf("%u", gdwDPlaySPRefCount);
    if (!oldDirectPlayCreate)
    {
        HMODULE hmod = LoadLibraryA("dplayx_old.dll");
        oldDirectPlayCreate = reinterpret_cast<decltype(oldDirectPlayCreate)>(
            GetProcAddress(hmod, "DirectPlayCreate"));    }
    HRESULT result =  oldDirectPlayCreate(lpGUID, lplpDP, pUnkOuter);
    
    return result;
}

HRESULT WINAPI DirectPlayEnumerateA(LPDPENUMDPCALLBACKA lpdpenumdpcallbackA, LPVOID lpVoid)
{
    printf("DirectPlayEnumerateA!\n");
    if (!oldDirectPlayEnumerateA)
    {
        HMODULE hmod = LoadLibraryA("dplayx_old.dll");
        oldDirectPlayEnumerateA = reinterpret_cast<decltype(oldDirectPlayEnumerateA)>(
            GetProcAddress(hmod, "DirectPlayEnumerateA"));
    }
    return oldDirectPlayEnumerateA(lpdpenumdpcallbackA, lpVoid);
}

HRESULT WINAPI DirectPlayEnumerateW(LPDPENUMDPCALLBACKW lpdpenumdpcallbackW, LPVOID lpVoid)
{
    printf("DirectPlayEnumerateW!\n");
    if (!oldDirectPlayEnumerateW)
    {
        HMODULE hmod = LoadLibraryA("dplayx_old.dll");
        oldDirectPlayEnumerateW = reinterpret_cast<decltype(oldDirectPlayEnumerateW)>(
            GetProcAddress(hmod, "DirectPlayEnumerateW"));
    }
    return oldDirectPlayEnumerateW(lpdpenumdpcallbackW, lpVoid);
}

HRESULT WINAPI DirectPlayLobbyCreateA(LPGUID lpGUID, LPDIRECTPLAYLOBBY* lpDPL,  IUnknown* pUnkOuter, LPVOID lpVoid, DWORD dwData)
{
    printf("DirectPlayLobbyCreateA!\n");
    if (!oldDirectPlayLobbyCreateA)
    {
        HMODULE hmod = LoadLibraryA("dplayx_old.dll");
        oldDirectPlayLobbyCreateA = reinterpret_cast<decltype(oldDirectPlayLobbyCreateA)>(
            GetProcAddress(hmod, "DirectPlayLobbyCreateA"));
    }
    return oldDirectPlayLobbyCreateA(lpGUID, lpDPL, pUnkOuter,lpVoid, dwData);
}

HRESULT WINAPI DirectPlayLobbyCreateW(LPGUID lpGUID, LPDIRECTPLAYLOBBY* lpDPL,  IUnknown* pUnkOuter, LPVOID lpVoid, DWORD dwData)
{
    printf("DirectPlayLobbyCreateW!\n");
    if (!oldDirectPlayLobbyCreateW)
    {
        HMODULE hmod = LoadLibraryA("dplayx_old.dll");
        oldDirectPlayLobbyCreateW = reinterpret_cast<decltype(oldDirectPlayLobbyCreateW)>(
            GetProcAddress(hmod, "DirectPlayLobbyCreateW"));
    }
    return oldDirectPlayLobbyCreateW(lpGUID, lpDPL, pUnkOuter,lpVoid, dwData);
}