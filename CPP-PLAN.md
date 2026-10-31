# kedi-kilit — C++ sürümü planı

Mevcut çalışan sürüm: `kedi-kilit.ahk` (AutoHotkey v2) — referans davranış.

## Kararlar (2026-10-31)
- Toolchain: **MSVC + CMake**, x64, tek `.exe`, harici bağımlılık yok
- LED: SDK `E:\Downloads\LED_SDK_9.00\LED\Lib\x64\LogitechLEDLib.lib` + `Include\LogitechLEDLib.h` statik link
  - .lib, G HUB DLL'ini registry CLSID ile bulup imzasını doğrulayıp `LoadLibraryW` ile yüklüyor → ek link: `crypt32.lib`, `imagehlp.lib`
  - MSVC ile derlenmiş, C++ mangled → MinGW ile linklenmez
- Yetki (UIPI): Görev Zamanlayıcı, oturum açılışında, **en yüksek ayrıcalıklarla**
- İlk sürüm kapsamı:
  1. Kilit / kilit açma (kod ile)
  2. Kedi algılayınca otomatik kilit
  3. Boşta kalınca otomatik kilit
  4. Ekran overlay'i
  5. Tepsi menüsü + `.ini` ayar dosyası

## Mevcut davranış (AHK'den taşınacak)
| | |
|---|---|
| Kilitle | `Pause` veya `Ctrl+Alt+L` |
| Aç | Kod (üst sıra veya numpad rakamları), araya giren rakam dışı tuş tamponu sıfırlar |
| Kilitliyken | Tüm tuşlar + fare tuşları/tekerlek/hareket yutulur |
| LED kilitli | `LogiLedInitWithName` → 400 ms → `LogiLedFlashLighting(100,0,0, 0=sonsuz, 400)` |
| LED açılış | `LogiLedStopEffects` → yeşil 500 ms → `LogiLedShutdown` (G HUB profiline döner) |
| Hedef cihaz | `LogiLedSetTargetDevice(LOGI_DEVICETYPE_ALL = 7)` |

## Mimari
- `WH_KEYBOARD_LL` + `WH_MOUSE_LL`, kilitliyken `return 1` (WM_MOUSEMOVE dahil → imleç donar)
- Hook callback **minimal**: durum güncelle + `PostMessage`; LED/overlay/IO ana mesaj döngüsünde
  - Sebep: `LowLevelHooksTimeout` aşılırsa Windows hook'u sessizce kaldırır
- Kod eşleşme: `vkCode` 0x30–0x39 ve `VK_NUMPAD0–9`; NumLock kapalı numpad için `scanCode` + `LLKHF_EXTENDED` kontrolü
- Kedi algılama: eşzamanlı basılı tuş sayısı ≥ N veya kısa pencerede uzak tuşlar (eşikler `.ini`)
- Boşta kilit: `GetLastInputInfo`, timer ile
- Overlay: `WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOPMOST | WS_EX_TOOLWINDOW`, tüm monitörler
- Tepsi: `Shell_NotifyIcon`, menü: Kilitle / Ayarlar / Çıkış
- Engellenemez (beklenen): Ctrl+Alt+Del, Win+L (SAS)

## Güvenlik ağı
- Debug build: kilitlendikten 30 sn sonra otomatik açılan watchdog (Release'te yok)
- Kod `.ini`'de; loglara yazılmaz
