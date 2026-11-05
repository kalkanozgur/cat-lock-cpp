# Durum

## Tamamlanan

- **Adım 0 — İlk kurulum (31.10.2026):** kurulum notları `docs/kurulum.md`.
- **Adım 1 — core (31.10.2026):** `config`, `key_map`, `code_matcher` + testler. Debug ve Release 0 uyarı, `kk_core_tests` 38/38.

- **Adım 2 — win (31.10.2026):** tepsi, ini, log, hook'lar, kilit/aç, watchdog, hook sağlık kontrolü. Kullanıcı Debug'da doğruladı.
- **Adım 4 — overlay (31.10.2026):** kullanıcı isteğiyle Adım 3'ten önce yapıldı, pati ikonları eklendi. Kullanıcı "şahane çalışıyor" dedi; çoklu monitör ve farklı DPI ayrıca teyit edilmedi. Debug ve Release 0 uyarı, `kk_core_tests` 41/41.
- **Adım 6 — kedi algılama (31.10.2026):** `cat_detector` + testler, tepsiden aç/kapa, tam ekran askısı. Debug ve Release 0 uyarı, `kk_core_tests` 75/75. Tepsiye karartma seviyesi ve tam ekran ayarı eklendi. Kullanıcı oyun testi dahil tüm manuel testleri doğruladı.
- **Adım 3 — LED (31.10.2026):** G HUB ve SDK yerine G213'e doğrudan HID (kullanıcı kararı): kırmızı nefes → 500 ms yeşil → `normal_color`. Kullanıcı Debug'da doğruladı. Cihaz açılışı 62–78 ms.
- **Adım 5 — boşta kilit (31.10.2026):** `idle_policy` + testler, varsayılan 3 dk, tepsiden Kapalı/1/3/5/10 dk. Kullanıcı Debug'da doğruladı. Debug ve Release 0 uyarı, `kk_core_tests` 89/89.

- **Adım 7 — kurulum (31.10.2026):** `scripts/register-task.ps1` (kur/güncelle/kaldır), Release, README. Kullanıcı testleri onayladı. Release 0 uyarı, ctest yeşil.

- **Yeniden adlandırma ve kaldırıcı (31.10.2026):** uygulama `cat-lock-cpp` oldu; ayar eski `KediKilit` klasöründen ilk açılışta taşınır. `scripts/uninstall.cmd` + `uninstall.ps1`, `dist` hedefi, Release boyut bayrakları eklendi. Kullanıcı testleri onayladı. 0 uyarı, `kk_core_tests` 89/89, Release exe 233 984 bayt.

- **Adım 8 — LED desenleri (05.11.2026):** kilit nefesi 2 s, açılışta 3× yeşil yanıp sönme (150 ms adım) → `normal_color`. Desenler `core/led_pattern.h`'de, cihazdan bağımsız. `CPP-PLAN.md` `docs/` altına taşındı. Kullanıcı Debug'da doğruladı. 0 uyarı, `kk_core_tests` 93/93.

## Sıradaki

- Sıradaki: **Adım 12 — fare (G502 HERO).**
- **Adım 9 — keşif (05.11.2026, tamam, salt okuma):**
  - Lenovo: ITE 8295 `048D:C963`, sayfa `FF89` usage `00CC`, 33 baytlık feature raporu → 4 bölge RGB. Vantage çalışmıyor.
  - Fare: G502 HERO `046D:C08B`, HID++ 2.0 uzun rapor (sayfa `FF00`, 20 bayt), 2 bölge. G HUB ajanı çalışmıyor.
  - `25A7:FA67` (CX 2.4G alıcı) aydınlatma cihazı değil, kapsam dışı.
- **Adım 10 — LED arka uç arayüzü (05.11.2026, tamam):** `win/hid.*` ortak HID, `win/led_device.h` arayüz, `win/led_g213.cpp`; `led.cpp` bulunan tüm cihazlara aynı deseni gönderir, hata cihaz başına bir kez loglanır. 0 uyarı, 93/93.
- **Adım 11 — Lenovo klavye (05.11.2026, tamam):** `core/lenovo_4zone.h` (feature 0xCC, 33 bayt) + testler, `win/led_lenovo.cpp`, `hid::set_feature`. PID değil koleksiyon (FF89/00CC) eşleşir. Nefes hız 2 (15 s sayım: ≈ 2–2,5 s); G213 2000 ms kaldı. Mevcut renk okunamaz (GetFeature cihaz bilgisi döndürür). Kullanıcı doğruladı. 0 uyarı, 97/97. Açık: G213–Lenovo uzun süreli kayma ölçülmedi; gerekirse hız 2 süresi 60 s sayımla kesinleştirilir.
- **Adım 12 — fare:** logo + DPI şeridi; açılışta `normal_color`.- Farklı DPI testi kullanıcı kararıyla kapsam dışı.
## Açık sorular

- Güncel AHK (`E:\Downloads\LED_SDK_9.00\kedi-kilit.ahk`, rakam kodlu) ile `legacy/kedi-kilit.ahk` (eski, harf parolalı) farklı. CLAUDE.md'nin "çalışan davranış örneği" bu klasördeki eski sürüm değil; referans CPP-PLAN.md.
- Aynı SDK klasöründeki imzasız `kedi-kilit.exe` (30.10.2026) incelenmedi ve çalıştırılmadı.

## Varsayımlar ve kararlar

- Repo: `kalkanozgur/cat-lock-cpp` (private), kullanıcı kararı. Push yalnız kullanıcı isteyince.
- Release private kalır (kullanıcı kararı), `v1.0.0` yayımlandı (31.10.2026). Yerel klasör adı `kedi-kilit` kaldı. Deny kuralı exe adını içeren her komutu engellediği için release'e zip yüklenir.
- Repoda olmayanlar (`.gitignore`), kullanıcı kararı: `kedi-kilit.ahk`, `LogitechLedEnginesWrapper.dll`, `third_party/logitech_led/`, kök dizindeki oturum dosyaları. Sonuç: yeni klonda `legacy/` yok. SDK artık kullanılmıyor; derleme ona bağlı değil.
- Generator `Visual Studio 18 2026` (Build Tools 18.5, MSVC 14.50, CMake 4.4.4).
- Kod sondan eşleşir (AHK davranışı): öndeki yanlış rakamlar açılışı engellemez.
- Tuş tekrarı ayıklanmaz: bırakma olayı kaybolursa (Ctrl+Alt+Del) o rakam yazılamaz, kilit açılamazdı.
- Numpad, VK tanınmazsa scan koduyla da tanınır (extended olmayan).
- Ayar aralıkları: `cat_min_simultaneous` 3–32, `cat_min_distinct` 3–64, `cat_window_ms` 50–5000, `idle_lock_min` 0–1440, `alpha` 0–255. Aralık dışı değer varsayılanda kalır.
- Overlay (Adım 4): kilitliyken tüm monitörler siyah katmanla kararır. %50 kullanıcıya fazla geldi; varsayılan `alpha` 77 (yaklaşık %30).
- Kedi algılamada Space ve WASD de modifier gibi sayılmaz (kullanıcı isteği, oyunlar için). Bedeli: kedi yalnız bu tuşlara basarsa algılanmaz.
- Tepsi ikonu kullanıcı isteğiyle: açıkken kedi patisi, kilitliyken üstü çizili pati. Dosyalar `src/win/res/`, üretici `scripts/make-icons.ps1`.
- Overlay kilitte kurulur, açılışta yok edilir; `WM_DISPLAYCHANGE` ile monitör değişiminde yeniden kurulur. Exclusive tam ekran oyunlarda görünmeyebilir.
- `ConfigIssue` değer ve bilinmeyen anahtar adı taşımaz; kod loga geçemez.
- Satır içi yorum desteklenmez.
- G213 protokolü belgesiz; bayt düzeni `G213Colors` ve `g810-led`'den, GPL kod kopyalanmadı. Mevcut renk okunamaz, açılışta `normal_color` yazılır. Uygulama çökerse klavye kırmızıda kalabilir.
- Boşta kilit, boşta kalma başına bir kez tetiklenir; yeni girdi gelince yeniden kurulur. Debug'da watchdog açtıktan sonra girdi olmadan yeniden kilitlemez: yoksa 30 s kilit döngüsüne girerdi. Release'te watchdog yok.
