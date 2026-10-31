#include "win/paths.h"

#include <windows.h>

#include <shlobj.h>

namespace kk::win {

namespace {

// Yeni ini yoksa eski kurulumun ayarını taşır. Var olan yeni ini'nin üzerine asla yazmaz.
// Eski log açıksa (eski sürüm çalışıyor) yerinde kalır; klasör yalnız boşsa silinir.
bool migrate_legacy(const std::wstring& base, const Paths& paths) {
    if (GetFileAttributesW(paths.ini.c_str()) != INVALID_FILE_ATTRIBUTES) {
        return false;
    }
    const std::wstring old_dir = base + L"\\KediKilit";
    const std::wstring old_ini = old_dir + L"\\kedi-kilit.ini";
    const std::wstring old_log = old_dir + L"\\kedi-kilit.log";
    if (!MoveFileExW(old_ini.c_str(), paths.ini.c_str(), MOVEFILE_COPY_ALLOWED)) {
        return false;
    }
    MoveFileExW(old_log.c_str(), paths.log.c_str(), MOVEFILE_COPY_ALLOWED);
    RemoveDirectoryW(old_dir.c_str());
    return true;
}

}  // namespace

bool resolve_paths(Paths& out) {
    PWSTR known = nullptr;
    if (FAILED(SHGetKnownFolderPath(FOLDERID_LocalAppData, KF_FLAG_DEFAULT, nullptr, &known))) {
        return false;
    }
    const std::wstring base(known);
    CoTaskMemFree(known);

    out.dir = base + L"\\CatLockCpp";
    if (!CreateDirectoryW(out.dir.c_str(), nullptr) && GetLastError() != ERROR_ALREADY_EXISTS) {
        return false;
    }
    out.ini = out.dir + L"\\cat-lock-cpp.ini";
    out.log = out.dir + L"\\cat-lock-cpp.log";
    out.migrated = migrate_legacy(base, out);
    return true;
}

}  // namespace kk::win
