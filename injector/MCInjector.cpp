#include <windows.h>
#include <tlhelp32.h>
#include <tchar.h>
#include <iostream>
#include <fstream>
#include "resource.h"

DWORD GetProcessID(const wchar_t* procName) {
    PROCESSENTRY32 entry = { 0 };
    entry.dwSize = sizeof(PROCESSENTRY32);
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (Process32First(snapshot, &entry)) {
        do {
            if (!_wcsicmp(entry.szExeFile, procName)) {
                DWORD pid = entry.th32ProcessID;
                CloseHandle(snapshot);
                return pid;
            }
        } while (Process32Next(snapshot, &entry));
    }

    CloseHandle(snapshot);
    return 0;
}

bool ExtractEmbeddedDll(const std::wstring& outputPath) {
    HMODULE hModule = GetModuleHandle(NULL);
    if (!hModule) {
        std::cout << "[!] GetModuleHandle failed\n";
        return false;
    }

    HRSRC hRes = FindResourceW(hModule, MAKEINTRESOURCEW(IDR_DLL1), RT_RCDATA);
    if (!hRes) {
        std::cout << "[!] FindResource failed\n";
        return false;
    }

    HGLOBAL hLoadedRes = LoadResource(hModule, hRes);
    if (!hLoadedRes) {
        std::cout << "[!] LoadResource failed\n";
        return false;
    }

    void* pLockedRes = LockResource(hLoadedRes);
    DWORD resSize = SizeofResource(hModule, hRes);
    if (!pLockedRes || resSize == 0) {
        std::cout << "[!] Lock/SizeofResource failed\n";
        return false;
    }

    std::ofstream outFile(outputPath, std::ios::binary);
    if (!outFile) {
        std::cout << "[!] Failed to open output file\n";
        return false;
    }

    outFile.write((char*)pLockedRes, resSize);
    outFile.close();

    std::wstring ws = L"Test";
    std::string str(ws.begin(), ws.end()); 
    std::cout << str << std::endl;
    return true;
}

bool InjectDLL(DWORD pid, const std::string& dllPath) {
    HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (!hProc) {
        std::cout << "[!] OpenProcess failed\n";
        return false;
    }

    void* alloc = VirtualAllocEx(hProc, NULL, dllPath.length() + 1, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!alloc) {
        std::cout << "[!] VirtualAllocEx failed\n";
        CloseHandle(hProc);
        return false;
    }

    WriteProcessMemory(hProc, alloc, dllPath.c_str(), dllPath.length() + 1, NULL);

    HANDLE hThread = CreateRemoteThread(hProc, NULL, 0,
        (LPTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryA"),
        alloc, 0, NULL);

    if (!hThread) {
        std::cout << "[!] CreateRemoteThread failed\n";
        CloseHandle(hProc);
        return false;
    }

    std::cout << "[+] DLL injected successfully!\n";
    CloseHandle(hThread);
    CloseHandle(hProc);
    return true;
}

int main() {
    AllocConsole();
    FILE* fp;
    freopen_s(&fp, "CONOUT$", "w", stdout);
    std::wcout << L"[+] McInjector started\n";

    const wchar_t* targetProc = L"javaw.exe";
    DWORD pid = GetProcessID(targetProc);

    if (pid == 0) {
        std::cout << "[!] Could not find javaw.exe\n";
        MessageBoxA(NULL, "Could not find Minecraft (javaw.exe)", "McInjector", MB_OK | MB_ICONERROR);
        return 1;
    }

    std::wstring tempPath = L"C:\\Users\\njohn\\AppData\\Local\\Temp\\McClientExtracted.dll";

    if (!ExtractEmbeddedDll(tempPath)) {
        std::cout << "[!] Failed to extract DLL\n";
        MessageBoxA(NULL, "Failed to extract embedded DLL", "McInjector", MB_OK | MB_ICONERROR);
        return 1;
    }

    if (!InjectDLL(pid, std::string(tempPath.begin(), tempPath.end()))) {
        std::cout << "[!] DLL injection failed\n";
        MessageBoxA(NULL, "DLL Injection failed", "McInjector", MB_OK | MB_ICONERROR);
        return 1;
    }

    std::cout << "[+] Done!\n";
    MessageBoxA(NULL, "McClient successfully injected!", "McInjector", MB_OK);
    return 0;
}
