# kedi-kilit

Kediler klavyeye bastığında klavye ve fareyi kilitleyen Windows tepsi uygulaması. Kilitliyken Logitech G213 kırmızı yanıp söner; rakam koduyla açılır.

> **Durum:** geliştirme aşamasında — güncel adım `docs/DURUM.md`. Çalışan sürüm: `legacy/kedi-kilit.ahk` (AutoHotkey v2).

## Özellikler

| Özellik | Davranış |
|---|---|
| Elle kilit | `Pause` veya `Ctrl+Alt+L` |
| Açma | Rakam kodu — üst sıra veya numpad (NumLock kapalıyken de) |
| Kedi algılama | Çok tuşa aynı anda ya da insanüstü hızda basılınca otomatik kilit |
| Boşta kilit | N dakika girdi yoksa (varsayılan kapalı) |
| Overlay | Kilitliyken tüm monitörler yaklaşık %30 kararır |
| LED | Kilitli: kırmızı yanıp söner · Açılış: 0,5 s yeşil → G HUB profiline döner |
| Tepsi | Kilitle · Kedi algılama aç/kapa · Ayar dosyası · Log klasörü · Çıkış |

## Gereksinimler

| Bileşen | Not |
|---|---|
| Windows 10/11 x64 | |
| Visual Studio 2022 / 2026 veya Build Tools | "Desktop development with C++" iş yükü (MSVC + Windows SDK). MinGW çalışmaz (SDK `.lib` MSVC'ye özel) |
| CMake ≥ 3.21 (VS 2026 için ≥ 4.2) | `winget install Kitware.CMake` — VS içindeki CMake terminal PATH'inde değil |
| Git for Windows | `winget install Git.Git` |
| VS Code + CMake Tools + C/C++ eklentileri | `.vscode/extensions.json` önerir |
| Claude Code (CLI) | Derleme ve testleri yürütür |
| Logitech G HUB | LED için; G HUB'da uygulamaların aydınlatmayı kontrol etmesine izin verilmeli |
| Logitech LED SDK 9.00 | `third_party/logitech_led/` — ilk kurulumda `E:\Downloads\LED_SDK_9.00`'dan kopyalanır |

## İlk açılış

1. `README.md` ve `CLAUDE.md`'yi `E:\Documents\kedi-kilit\` içine koy (`CPP-PLAN.md`, `kedi-kilit.ahk`, `claude-kurallar.md`, `LogitechLedEnginesWrapper.dll` zaten orada).
2. VS Code → **File → Open Folder** → `E:\Documents\kedi-kilit`.
3. Terminal (`Ctrl+\``) → `claude` → `İlk kurulumu yap`.
4. Claude Code ortamı kontrol eder, klasörü düzenler, iskeleti kurar, derler, test eder ve raporlar. Onayınla ilk commit'i atar.
5. Sonraki her adım için yeni oturum: `claude` → `Sıradaki adım`. Adım listesi: `CLAUDE.md` §8.

## Derleme

Normalde Claude Code yürütür. Elle:

```powershell
cmake --preset msvc-x64
cmake --build --preset debug      # veya: --preset release
ctest --preset debug
```

VS Code: alt çubuktan preset seç → **Build**. Çıktı: `build\Debug\kedi-kilit.exe`, `build\Release\kedi-kilit.exe`.

> ⚠️ Claude Code `kedi-kilit.exe`'yi **asla** çalıştırmaz — kilitlenirsin ve o açamaz. Çalıştırma testleri sende. İlk testleri **Debug** ile yap: 30 s sonra kendiliğinden açılır.

## Kurulum (oturum açılışında, yüksek yetkiyle) — Adım 7'de eklenir

Yönetici PowerShell:

```powershell
.\scripts\register-task.ps1            # kur
.\scripts\register-task.ps1 -Remove    # kaldır
```

- Release exe'yi `C:\Program Files\KediKilit\`'e kopyalar, Görev Zamanlayıcı'ya "oturum açılışında, en yüksek ayrıcalıklarla" görev ekler.
- Yüksek yetki gerekçesi: yönetici olarak çalışan bir pencere odaktayken normal yetkili hook tuşları göremez (UIPI).
- Exe `Program Files`'ta: yükseltilmiş görev, kullanıcının değiştirebileceği bir yoldan çalışmaz; ayrıca çalışırken yeniden derleme çakışmaz.

## Ayarlar — `%LOCALAPPDATA%\KediKilit\kedi-kilit.ini`

Şablon: `config/kedi-kilit.example.ini`. **Kod tanımlı değilse uygulama kilitlemez.**

| Bölüm / anahtar | Varsayılan | Açıklama |
|---|---|---|
| `[lock] code` | — | 4–16 rakam |
| `[lock] hotkey` | `both` | `pause` / `ctrl_alt_l` / `both` |
| `[auto] cat_detect` | `1` | Kedi algılama |
| `[auto] cat_min_simultaneous` | `4` | Modifier, Space ve WASD hariç eşzamanlı basılı tuş |
| `[auto] cat_window_ms` | `250` | Hız penceresi |
| `[auto] cat_min_distinct` | `6` | Pencere içinde farklı tuş sayısı |
| `[auto] idle_lock_min` | `0` | Boşta kilit süresi, `0` = kapalı |
| `[auto] pause_in_fullscreen` | `1` | Tam ekran oyun/sunumda otomatik kilit yok |
| `[overlay] enabled` / `alpha` | `1` / `77` | Karartma: `0` yok, `77` ≈ %30, `128` ≈ %50, `255` tam siyah |
| `[led] enabled` | `1` | |

Log: `%LOCALAPPDATA%\KediKilit\kedi-kilit.log` — kilit/açma olayları ve sebepleri, LED durumu. Kod ve basılan tuşlar yazılmaz.

Kod `.ini`'de düz metin: tehdit modeli kediler; kişilere karşı bir güvenlik aracı değildir.

## Sınırlar

- `Ctrl+Alt+Del` ve `Win+L` engellenemez (Windows güvenli giriş dizisi).
- Kodu unutursan: `Ctrl+Alt+Del` → **Oturumu kapat** (Görev Yöneticisi işe yaramaz; normal masaüstündeki girdi hâlâ yutulur).
- G HUB kapalıysa LED çalışmaz; kilit çalışır.

## Sorun giderme

| Belirti | Sebep / çözüm |
|---|---|
| LED tepki vermiyor | G HUB açık mı · aydınlatma kontrol izni · log'da `LED init` satırı |
| Yönetici penceresi odaktayken kısayol çalışmıyor | Uygulama yükseltilmiş değil → Görev Zamanlayıcı ile başlat |
| Kilit kendiliğinden düşüyor | Hook zaman aşımı; log'da `hook reinstall` satırı → bildir |
| Oyunda yanlışlıkla kilitlendi | `pause_in_fullscreen=1` · eşikleri artır · tepsiden kedi algılamayı kapat |
| `LNK1104: cannot open kedi-kilit.exe` | Debug exe açık → tepsiden **Çıkış** |

## Klasör yapısı

```
kedi-kilit/
  CMakeLists.txt  CMakePresets.json  README.md  CLAUDE.md  CPP-PLAN.md
  src/core/      platformdan bağımsız mantık (testli)
  src/win/       Win32: hook, LED, overlay, tepsi
  tests/         kk_core_tests
  config/        kedi-kilit.example.ini
  scripts/       register-task.ps1
  third_party/   logitech_led (SDK header + x64 .lib)
  legacy/        kedi-kilit.ahk, LogitechLedEnginesWrapper.dll
  docs/          DURUM.md, kurulum.md, claude-kurallar.md
  .claude/       settings.json, skills/oturum-logu
  .vscode/       settings.json, extensions.json
```

`third_party/logitech_led/` ve `legacy/` yalnız yerelde durur, repoya girmez (Logitech lisansı, parola). Yeni klonda SDK'nın `LogitechLEDLib.h` ve x64 `LogitechLEDLib.lib` dosyaları elle `third_party/logitech_led/` içine kopyalanır; eksikse CMake açık hatayla durur.
