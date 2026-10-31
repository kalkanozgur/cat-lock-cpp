#pragma once
#include <string>

namespace kk::win {

struct Paths {
    std::wstring dir;  // %LOCALAPPDATA%\CatLockCpp
    std::wstring ini;
    std::wstring log;
    bool migrated = false;  // Ayar eski KediKilit klasöründen taşındı.
};

// Klasörü de oluşturur, eski kurulumun ayarını taşır. Bulunamazsa false.
bool resolve_paths(Paths& out);

}  // namespace kk::win
