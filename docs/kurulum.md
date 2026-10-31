# İlk kurulum (tamamlandı — referans)

> 31.10.2026'da uygulandı. Sapmalar: generator `Visual Studio 18 2026`; repo önce GitHub'a yüklendi (`kalkanozgur/cat-lock-cpp`, private); AHK, DLL, SDK ve kök dizindeki oturum dosyaları kullanıcı kararıyla `.gitignore`'da. Yeni klonda SDK `third_party/logitech_led/` içine elle kopyalanır.


Klasörde beklenen: `kedi-kilit.ahk`, `LogitechLedEnginesWrapper.dll`, `CPP-PLAN.md`, `claude-kurallar.md`, `README.md`, `CLAUDE.md`. Dosya taşıma/silme öncesi listeyi göster, tek onay al.

1. **Ortam kontrolü** — sonucu tabloyla raporla; eksik varsa dur ve README "Gereksinimler"e yönlendir:
   - `cmake --version` → ≥ 3.21 (VS 2026 için ≥ 4.2). VS içindeki CMake PATH'te değildir.
   - VS sürümü: `& "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationVersion` → 17.x = `Visual Studio 17 2022`, 18.x = `Visual Studio 18 2026`
   - `git --version`
   - `E:\Downloads\LED_SDK_9.00\LED\Include\LogitechLEDLib.h` ve `E:\Downloads\LED_SDK_9.00\LED\Lib\x64\LogitechLEDLib.lib` mevcut mu
2. **Düzenle:** `kedi-kilit.ahk` + `LogitechLedEnginesWrapper.dll` → `legacy/`; `claude-kurallar.md` → `docs/`. Oturum logu dosyaları (`20??-??-??_*.md`, `*_usage.json`) varsa kullanıcıya sor (`E:\Documents\Claude-Logs`'a taşı / bırak). `legacy/kedi-kilit.ahk` harf parolası (`PASSWORD :=`) içeriyorsa eski sürümdür → bildir; referans CPP-PLAN.md.
3. **SDK:** header + x64 `.lib` (+ SDK'daki lisans/readme) → `third_party/logitech_led/`.
4. **Dosyalar:**
   - `CMakePresets.json` — aşağıdaki şablon, generator 1. adımdan.
   - `CMakeLists.txt` — C++20; `kk_core` static; `kedi-kilit` WIN32 exe (boş `WinMain` + `app.manifest`); `kk_core_tests`; `enable_testing()` + `add_test`; §3 bayrakları; `CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"`; `KK_WATCHDOG` yalnız Debug; SDK `SYSTEM` include + link.
   - `tests/harness.h` (~40 satır: `TEST(ad)` kaydı, `CHECK` / `CHECK_EQ`, sayaç, çıkış kodu) + 1 örnek test.
   - `config/kedi-kilit.example.ini` — README "Ayarlar" tablosundaki anahtarlar, `code=` boş.
   - `.gitignore` — `build/`, `out/`, `.vs/`, `*.user`, `*.ini`, `!config/*.example.ini`.
   - `.vscode/settings.json` — `"cmake.useCMakePresets": "always"`, `"C_Cpp.default.configurationProvider": "ms-vscode.cmake-tools"`; `.vscode/extensions.json` — `ms-vscode.cmake-tools`, `ms-vscode.cpptools`.
   - `.claude/settings.json` — `permissions.additionalDirectories: ["E:\\Documents\\Claude-Logs"]`; `permissions.deny`: `kedi-kilit.exe`'yi başlatan komutlar. Kural sözdizimini kurulu Claude Code sürümüne göre doğrula; **exe'yi çalıştırarak test etme**. Test exe'si `kk_core_tests.exe` desenle eşleşmemeli.
   - `.claude/skills/oturum-logu/SKILL.md` — `docs/claude-kurallar.md` Bölüm 7'den uyarla: yazma doğrudan dosyaya (`device_*` araçları yok); klasör erişimi `additionalDirectories`; **"Oturum sonu kullanım özeti" bölümü ve Python script'i çıkar** (PC'de yerel Claude Code → usage widget dökümü kendisi okur, çift sayım); saat aşağıdaki komuttan.
   - `docs/DURUM.md` — tamamlanan: Adım 0 · sıradaki: Adım 1 · açık sorular · varsayımlar.
5. **Genel kurallar:** `docs/claude-kurallar.md` Bölüm 1–5 → `%USERPROFILE%\.claude\CLAUDE.md`. Dosya varsa üzerine yazma: farkı göster, onayla ekle. Uyarlamalar: zaman damgası aşağıdaki komutla; "2 dk sonra hatırlat" → "adım sonunda tekrar sor"; Bölüm 4 → "projede `oturum-logu` becerisi varsa onu kullan".
6. **Derle:** `cmake --preset msvc-x64` → `cmake --build --preset debug` → `ctest --preset debug`. Exe'yi çalıştırma.
7. **Git:** `git init` → rapor → onay → ilk commit `chore: proje iskeleti`.
8. Bu bölümü `docs/kurulum.md`'ye taşı (yukarıdaki nota göre).

Zaman damgası (`🕒 Gün GG Ay YYYY · SS:DD (TSİ)`):

```powershell
(Get-Date).ToString('ddd dd MMM yyyy · HH:mm', [Globalization.CultureInfo]'tr-TR') + ' (TSİ)'
```

`CMakePresets.json` şablonu:

```json
{
  "version": 3,
  "configurePresets": [{
    "name": "msvc-x64",
    "generator": "Visual Studio 17 2022",
    "architecture": { "value": "x64", "strategy": "set" },
    "binaryDir": "${sourceDir}/build"
  }],
  "buildPresets": [
    { "name": "debug",   "configurePreset": "msvc-x64", "configuration": "Debug" },
    { "name": "release", "configurePreset": "msvc-x64", "configuration": "Release" }
  ],
  "testPresets": [
    { "name": "debug", "configurePreset": "msvc-x64", "configuration": "Debug",
      "output": { "outputOnFailure": true } }
  ]
}
```

Visual Studio generator vcvars / Developer PowerShell gerektirmez → düz terminalden çalışır; VS Code CMake Tools aynı preset'leri okur.
