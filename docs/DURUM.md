# Durum

## Tamamlanan

- **Adım 0 — İlk kurulum (31.10.2026):** kurulum notları `docs/kurulum.md`.
- **Adım 1 — core (31.10.2026):** `config`, `key_map`, `code_matcher` + testler. Debug ve Release 0 uyarı, `kk_core_tests` 38/38.

- **Adım 2 — win (31.10.2026):** tepsi, ini, log, hook'lar, kilit/aç, watchdog, hook sağlık kontrolü. Kullanıcı Debug'da doğruladı.
- **Adım 4 — overlay (31.10.2026):** kullanıcı isteğiyle Adım 3'ten önce yapıldı, pati ikonları eklendi. Kullanıcı "şahane çalışıyor" dedi; çoklu monitör ve farklı DPI ayrıca teyit edilmedi. Debug ve Release 0 uyarı, `kk_core_tests` 41/41.
- **Adım 6 — kedi algılama (31.10.2026):** `cat_detector` + testler, tepsiden aç/kapa, tam ekran askısı. Debug ve Release 0 uyarı, `kk_core_tests` 74/74. Kullanıcı isteğiyle commit edildi; bitiş koşulu "oyunda yanlış tetik yok" henüz doğrulanmadı.

## Sıradaki

- **Adım 3:** LED. Bitti sayılır: kırmızı yanıp sönme → yeşil → G HUB profili.

## Açık sorular

- Güncel AHK (`E:\Downloads\LED_SDK_9.00\kedi-kilit.ahk`, rakam kodlu) ile `legacy/kedi-kilit.ahk` (eski, harf parolalı) farklı. CLAUDE.md'nin "çalışan davranış örneği" bu klasördeki eski sürüm değil; referans CPP-PLAN.md.
- Aynı SDK klasöründeki imzasız `kedi-kilit.exe` (30.10.2026) incelenmedi ve çalıştırılmadı.

## Varsayımlar ve kararlar

- Repo: `kalkanozgur/cat-lock-cpp` (private), kullanıcı kararı. Push yalnız kullanıcı isteyince.
- Repoda olmayanlar (`.gitignore`), kullanıcı kararı: `kedi-kilit.ahk`, `LogitechLedEnginesWrapper.dll`, `third_party/logitech_led/`, kök dizindeki oturum dosyaları. Sonuç: yeni klonda `legacy/` ve SDK yok; CMake eksik SDK'da açık hatayla durur.
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
