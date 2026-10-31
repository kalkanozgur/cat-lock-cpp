#pragma once
#include <string>

namespace kk::win {

struct Paths {
    std::wstring dir;  // %LOCALAPPDATA%\KediKilit
    std::wstring ini;
    std::wstring log;
};

// Klasörü de oluşturur. Bulunamazsa false.
bool resolve_paths(Paths& out);

}  // namespace kk::win
