#include "win/log.h"

#include <windows.h>

#include <cstdio>

namespace kk::win {

namespace {
std::wstring g_path;
}

void log_init(std::wstring path) {
    g_path = std::move(path);
}

void log_line(std::string_view message) {
    if (g_path.empty()) {
        return;
    }
    SYSTEMTIME t;
    GetLocalTime(&t);
    char stamp[32];
    const int stamp_len = std::snprintf(stamp, sizeof(stamp), "%04u-%02u-%02u %02u:%02u:%02u ",
                                        t.wYear, t.wMonth, t.wDay, t.wHour, t.wMinute, t.wSecond);
    std::string line(stamp, static_cast<std::size_t>(stamp_len));
    line.append(message);
    line.push_back('\n');

    const HANDLE file = CreateFileW(g_path.c_str(), FILE_APPEND_DATA, FILE_SHARE_READ | FILE_SHARE_WRITE,
                                    nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file == INVALID_HANDLE_VALUE) {
        return;
    }
    DWORD written = 0;
    WriteFile(file, line.data(), static_cast<DWORD>(line.size()), &written, nullptr);
    CloseHandle(file);
}

}  // namespace kk::win
