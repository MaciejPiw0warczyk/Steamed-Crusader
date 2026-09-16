#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <windows.h>
#include "dplay.h"
#include "dplobby.h"
#include <thread>

using MySendTo = int (WINAPI *)(SOCKET s, const char* buf, int len, int flags, const sockaddr* to, int tolen);
using MyRecvFrom = int (WINAPI *)(SOCKET s, char* buf, int len, int flags, sockaddr* from, int* fromlen);

static HRESULT (WINAPI*oldDirectPlayCreate)(LPGUID, LPDIRECTPLAY*, IUnknown*) = NULL;

static HRESULT (WINAPI*oldDirectPlayEnumerateA)(LPDPENUMDPCALLBACKA, LPVOID) = NULL;
static HRESULT (WINAPI*oldDirectPlayEnumerateW)(LPDPENUMDPCALLBACKW, LPVOID) = NULL;

static HRESULT (WINAPI*oldDirectPlayLobbyCreateA)(LPGUID, LPDIRECTPLAYLOBBY*, IUnknown*, LPVOID, DWORD) = NULL;
static HRESULT (WINAPI*oldDirectPlayLobbyCreateW)(LPGUID, LPDIRECTPLAYLOBBY*, IUnknown*, LPVOID, DWORD) = NULL;

static int (WINAPI *oldSendTo)(SOCKET, const char*, int, int, const sockaddr*, int) = NULL;

template<typename T>
void IATHook(LPCSTR moduleName, LPCSTR targetModuleName, DWORD functionToHook, T func);
static int WSAAPI hookedSendTo(SOCKET s, const char* buf, int len, int flags, const sockaddr* to, int tolen);
static void LookAndHookLibraryTask(LPCSTR moduleName);


static void GetDpwsockx();

extern "C" __declspec(dllexport) DWORD gdwDPlaySPRefCount = 0;
// increments by 1 every timne you create or serach for a lobby


__declspec(dllexport) auto DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) -> BOOL
{
    if (fdwReason == DLL_PROCESS_ATTACH)
    {
        AllocConsole();

        FILE* f;
        freopen_s(&f, "CONOUT$", "w", stdout);
        printf("dplayx.dll loaded!\n");
        
        std::string param = "dpwsockx.dll";
        std::thread t1(LookAndHookLibraryTask, "dpwsockx.dll");
        t1.detach();
    }
    return 1;
}


void LookAndHookLibraryTask(LPCSTR moduleName)
{
    while (!GetModuleHandleA(moduleName))
    {
        Sleep(100);
    }
    IATHook(moduleName, "ws2_32.dll", 20, hookedSendTo);    
}



HRESULT WINAPI DirectPlayCreate(LPGUID lpGUID, LPDIRECTPLAY* lplpDP, IUnknown* pUnkOuter)
{
    printf("DirectPlayCreate!\n");
    if (!oldDirectPlayCreate)
    {
        HMODULE hmod = LoadLibraryA("dplayx_old.dll");
        oldDirectPlayCreate = reinterpret_cast<decltype(oldDirectPlayCreate)>(
            GetProcAddress(hmod, "DirectPlayCreate"));
    }
    return oldDirectPlayCreate(lpGUID, lplpDP, pUnkOuter);
}

int WSAAPI hookedSendTo(SOCKET s, const char* buf, int len, int flags, const sockaddr* to, int tolen)
{
    printf("hookedSendTo!\n");
    if (!oldSendTo)
    {
        HMODULE hmod = LoadLibraryA("dpwsockx.dll");
        oldSendTo = reinterpret_cast<decltype(oldSendTo)>(
            GetProcAddress(hmod, "hookedSendTo"));
    }
    return oldSendTo(s, buf, len, flags, to, tolen);
}

template<typename T>
void IATHook(LPCSTR moduleName,LPCSTR targetModuleName,  DWORD ordinalToHook, T func)
{
    LPVOID imageBase = GetModuleHandleA(moduleName);
    
    if (!imageBase)
        return;
    
    auto dos = (PIMAGE_DOS_HEADER)((DWORD_PTR)imageBase);
    auto nt  = (PIMAGE_NT_HEADERS)((DWORD_PTR)imageBase + dos->e_lfanew);

    IMAGE_DATA_DIRECTORY importDir = nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
    auto imports = reinterpret_cast<PIMAGE_IMPORT_DESCRIPTOR>(importDir.VirtualAddress + (DWORD_PTR)imageBase);
    
    LPCSTR libName = nullptr;
    HMODULE lib = nullptr;
    PIMAGE_IMPORT_BY_NAME functionName = nullptr;
    
    while (imports->Name != NULL)
    {
        libName = reinterpret_cast<LPCSTR>(imports->Name + (DWORD_PTR)imageBase);
        lib = LoadLibraryA(libName);
        if (lib)
        {
            auto originalFirstThunk = reinterpret_cast<PIMAGE_THUNK_DATA>((DWORD_PTR)imageBase + imports->OriginalFirstThunk);
            auto firstThunk = reinterpret_cast<PIMAGE_THUNK_DATA>((DWORD_PTR)imageBase + imports->FirstThunk);
            
            while (originalFirstThunk->u1.AddressOfData!=NULL)
            {
                if (IMAGE_ORDINAL32(originalFirstThunk->u1.Ordinal) == ordinalToHook)
                {
                    DWORD oldProtect = 0;
                    VirtualProtect(&firstThunk->u1.Function,4,PAGE_READWRITE,&oldProtect);
                    firstThunk->u1.Function = reinterpret_cast<DWORD>(func);
                    VirtualProtect(&firstThunk->u1.Function,4,oldProtect,&oldProtect);
                }
                originalFirstThunk++;
                firstThunk++;
            }
        }
        imports++;
    }
}




//Unused passthroughs 
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

HRESULT WINAPI DirectPlayLobbyCreateA(LPGUID lpGUID, LPDIRECTPLAYLOBBY* lpDPL, IUnknown* pUnkOuter, LPVOID lpVoid,
                                      DWORD dwData)
{
    printf("DirectPlayLobbyCreateA!\n");
    if (!oldDirectPlayLobbyCreateA)
    {
        HMODULE hmod = LoadLibraryA("dplayx_old.dll");
        oldDirectPlayLobbyCreateA = reinterpret_cast<decltype(oldDirectPlayLobbyCreateA)>(
            GetProcAddress(hmod, "DirectPlayLobbyCreateA"));
    }
    return oldDirectPlayLobbyCreateA(lpGUID, lpDPL, pUnkOuter, lpVoid, dwData);
}

HRESULT WINAPI DirectPlayLobbyCreateW(LPGUID lpGUID, LPDIRECTPLAYLOBBY* lpDPL, IUnknown* pUnkOuter, LPVOID lpVoid,
                                      DWORD dwData)
{
    printf("DirectPlayLobbyCreateW!\n");
    if (!oldDirectPlayLobbyCreateW)
    {
        HMODULE hmod = LoadLibraryA("dplayx_old.dll");
        oldDirectPlayLobbyCreateW = reinterpret_cast<decltype(oldDirectPlayLobbyCreateW)>(
            GetProcAddress(hmod, "DirectPlayLobbyCreateW"));
    }
    return oldDirectPlayLobbyCreateW(lpGUID, lpDPL, pUnkOuter, lpVoid, dwData);
}