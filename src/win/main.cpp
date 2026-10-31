#include <windows.h>

#include <objbase.h>

#include "win/app.h"

int WINAPI WinMain(HINSTANCE instance, HINSTANCE, LPSTR, int) {
    // Tek instance. Yükseltilmiş kopya varken mutex'e erişim reddedilir: o da "var" demektir.
    const HANDLE mutex = CreateMutexW(nullptr, FALSE, L"Local\\CatLockCpp.SingleInstance");
    if (mutex == nullptr || GetLastError() == ERROR_ALREADY_EXISTS) {
        if (mutex != nullptr) {
            CloseHandle(mutex);
        }
        return 0;
    }

    // ShellExecute için.
    const HRESULT com = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    const int result = kk::win::run_app(instance);
    if (SUCCEEDED(com)) {
        CoUninitialize();
    }
    CloseHandle(mutex);
    return result;
}
