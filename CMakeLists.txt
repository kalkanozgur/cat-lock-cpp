cmake_minimum_required(VERSION 3.21)

# Statik CRT: exe harici DLL istemez (CLAUDE.md §2). project()'ten önce olmalı.
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

project(kedi-kilit LANGUAGES CXX)

if(NOT MSVC)
  message(FATAL_ERROR "Yalnız MSVC desteklenir: LogitechLEDLib.lib MSVC-mangled.")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(/W4 /WX /permissive- /utf-8 /EHsc /external:W0)

# Logitech SDK lisans nedeniyle repoda değil; yerelde kopyalanmış olmalı.
set(KK_LED_DIR "${CMAKE_SOURCE_DIR}/third_party/logitech_led")
if(NOT EXISTS "${KK_LED_DIR}/LogitechLEDLib.h" OR NOT EXISTS "${KK_LED_DIR}/LogitechLEDLib.lib")
  message(FATAL_ERROR
    "Logitech LED SDK eksik: LogitechLEDLib.h ve x64 LogitechLEDLib.lib dosyalarını "
    "${KK_LED_DIR} içine kopyala (kaynak: LED_SDK_9.00\\LED\\Include ve Lib\\x64).")
endif()

# core: Win32 include yok, testlenir.
add_library(kk_core STATIC
  src/core/code_matcher.cpp
  src/core/config.cpp
  src/core/key_map.cpp
)
target_include_directories(kk_core PUBLIC src)

# Uygulama. Hook'lar klavye ve fareyi yutar: CTest hedefi yapılmaz.
add_executable(kedi-kilit WIN32
  src/win/main.cpp
  src/win/app.manifest
)
target_include_directories(kedi-kilit SYSTEM PRIVATE "${KK_LED_DIR}")
target_compile_definitions(kedi-kilit PRIVATE
  UNICODE _UNICODE WIN32_LEAN_AND_MEAN NOMINMAX
  $<$<CONFIG:Debug>:KK_WATCHDOG>
)
target_link_libraries(kedi-kilit PRIVATE
  kk_core
  "${KK_LED_DIR}/LogitechLEDLib.lib"
  crypt32 imagehlp wintrust
)

# Testler: yalnız core, hook yok.
add_executable(kk_core_tests
  tests/test_main.cpp
  tests/test_code_matcher.cpp
  tests/test_config.cpp
  tests/test_key_map.cpp
)
target_include_directories(kk_core_tests PRIVATE tests)
target_compile_definitions(kk_core_tests PRIVATE
  KK_EXAMPLE_INI="${CMAKE_SOURCE_DIR}/config/kedi-kilit.example.ini"
)
target_link_libraries(kk_core_tests PRIVATE kk_core)

enable_testing()
add_test(NAME kk_core_tests COMMAND kk_core_tests)
