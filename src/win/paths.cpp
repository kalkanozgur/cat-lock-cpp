#include "win/paths.h"

#include <windows.h>

#include <shlobj.h>

namespace kk::win {

bool resolve_paths(Paths& out) {
    PWSTR base = nullptr;
    if (FAILED(SHGetKnownFolderPath(FOLDERID_LocalAppData, KF_FLAG_DEFAULT, nullptr, &base))) {
        return false;
    }
    out.dir = std::wstring(base) + L"\\KediKilit";
    CoTaskMemFree(base);

    if (!CreateDirectoryW(out.dir.c_str(), nullptr) && GetLastError() != ERROR_ALREADY_EXISTS) {
        return false;
    }
    out.ini = out.dir + L"\\kedi-kilit.ini";
    out.log = out.dir + L"\\kedi-kilit.log";
    return true;
}

}  // namespace kk::win
