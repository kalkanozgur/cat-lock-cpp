cmake_minimum_required(VERSION 3.21)

# Statik CRT: exe yalnız Windows DLL'lerine bağlıdır (CLAUDE.md §2). project()'ten önce olmalı.
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

project(cat-lock-cpp VERSION 1.0.0 LANGUAGES CXX RC)

if(NOT MSVC)
  message(FATAL_ERROR "Yalnız MSVC desteklenir.")
endif()

# Release: hız yerine boyut. Hook yolu kısa, /O1 davranışı değiştirmez.
set(CMAKE_CXX_FLAGS_RELEASE "/O1 /Ob1 /DNDEBUG /Gy /Gw /GL")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "/LTCG /OPT:REF /OPT:ICF /INCREMENTAL:NO")
set(CMAKE_STATIC_LINKER_FLAGS_RELEASE "/LTCG")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(/W4 /WX /permissive- /utf-8 /EHsc /external:W0)

# core: Win32 include yok, testlenir.
add_library(kk_core STATIC
  src/core/cat_detector.cpp
  src/core/code_matcher.cpp
  src/core/config.cpp
  src/core/key_map.cpp
)
target_include_directories(kk_core PUBLIC src)

# Uygulama. Hook'lar klavye ve fareyi yutar: CTest hedefi yapılmaz.
add_executable(cat-lock-cpp WIN32
  src/win/app.cpp
  src/win/hooks.cpp
  src/win/led.cpp
  src/win/log.cpp
  src/win/main.cpp
  src/win/overlay.cpp
  src/win/paths.cpp
  src/win/tray.cpp
  src/win/app.manifest
  src/win/res/app.rc
)

# "Ayar dosyasını aç" ilk seferde bu şablonu yazar; tek kaynak example.ini.
set(KK_EXAMPLE_INI_PATH "${CMAKE_SOURCE_DIR}/config/cat-lock-cpp.example.ini")
file(READ "${KK_EXAMPLE_INI_PATH}" KK_EXAMPLE_INI_TEXT)
file(CONFIGURE OUTPUT "${CMAKE_BINARY_DIR}/generated/example_ini.h"
  CONTENT "#pragma once\ninline constexpr char kExampleIni[] = R\"kkini(@KK_EXAMPLE_INI_TEXT@)kkini\";\n"
  @ONLY)
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${KK_EXAMPLE_INI_PATH}")

target_include_directories(cat-lock-cpp PRIVATE "${CMAKE_BINARY_DIR}/generated")
target_compile_definitions(cat-lock-cpp PRIVATE
  UNICODE _UNICODE WIN32_LEAN_AND_MEAN NOMINMAX
  $<$<CONFIG:Debug>:KK_WATCHDOG>
)
target_link_libraries(cat-lock-cpp PRIVATE
  kk_core
  hid setupapi
)

# Release paketi: cmake --build --preset release --target dist
# Debug'ta durur: watchdog'lu exe dağıtılmasın.
set(KK_DIST_NAME "cat-lock-cpp-${PROJECT_VERSION}-win-x64")
set(KK_DIST_DIR "${CMAKE_BINARY_DIR}/dist")
add_custom_target(dist
  COMMAND ${CMAKE_COMMAND} -E "$<IF:$<CONFIG:Release>,true,false>"
  COMMAND ${CMAKE_COMMAND} -E rm -rf "${KK_DIST_DIR}"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${KK_DIST_DIR}/${KK_DIST_NAME}"
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "$<TARGET_FILE:cat-lock-cpp>"
    "${CMAKE_SOURCE_DIR}/scripts/register-task.ps1"
    "${CMAKE_SOURCE_DIR}/scripts/uninstall.ps1"
    "${CMAKE_SOURCE_DIR}/scripts/uninstall.cmd"
    "${KK_EXAMPLE_INI_PATH}"
    "${KK_DIST_DIR}/${KK_DIST_NAME}"
  COMMAND ${CMAKE_COMMAND} -E chdir "${KK_DIST_DIR}"
    ${CMAKE_COMMAND} -E tar cf "${KK_DIST_NAME}.zip" --format=zip "${KK_DIST_NAME}"
  VERBATIM
)
add_dependencies(dist cat-lock-cpp)

# Testler: yalnız core, hook yok.
add_executable(kk_core_tests
  tests/test_main.cpp
  tests/test_cat_detector.cpp
  tests/test_code_matcher.cpp
  tests/test_config.cpp
  tests/test_g213.cpp
  tests/test_hook_health.cpp
  tests/test_idle_policy.cpp
  tests/test_key_map.cpp
)
target_include_directories(kk_core_tests PRIVATE tests)
target_compile_definitions(kk_core_tests PRIVATE
  KK_EXAMPLE_INI="${CMAKE_SOURCE_DIR}/config/cat-lock-cpp.example.ini"
)
target_link_libraries(kk_core_tests PRIVATE kk_core)

enable_testing()
add_test(NAME kk_core_tests COMMAND kk_core_tests)
