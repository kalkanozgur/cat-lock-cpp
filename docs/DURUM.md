# Durum

## Tamamlanan

- **Adım 0 — İlk kurulum (31.10.2026):** kurulum notları `docs/kurulum.md`.
- **Adım 1 — core (31.10.2026):** `config`, `key_map`, `code_matcher` + testler. Debug ve Release 0 uyarı, `kk_core_tests` 38/38.

## Sıradaki

- **Adım 2:** win: tepsi, ini, log, hook'lar, kilit/aç, watchdog, hook sağlık kontrolü. Bitti sayılır: kullanıcı Debug'da kilit/aç doğruladı.

## Açık sorular

- `kedi-kilit.exe` hiç çalıştırılmadı (yasak); ilk çalıştırma testi Adım 2'de kullanıcıda.
- Güncel AHK (`E:\Downloads\LED_SDK_9.00\kedi-kilit.ahk`, rakam kodlu) ile `legacy/kedi-kilit.ahk` (eski, harf parolalı) farklı. CLAUDE.md'nin "çalışan davranış örneği" bu klasördeki eski sürüm değil; referans CPP-PLAN.md.
- Aynı SDK klasöründeki imzasız `kedi-kilit.exe` (30.10.2026) incelenmedi ve çalıştırılmadı.
- Adım 2: basılı tutulan tuşun tekrarları hook'ta ayıklansın mı? `CodeMatcher` her KEYDOWN'u rakam sayıyor.

## Varsayımlar ve kararlar

- Repo: `kalkanozgur/cat-lock-cpp` (private), kullanıcı kararı. Push yalnız kullanıcı isteyince.
- Repoda olmayanlar (`.gitignore`), kullanıcı kararı: `kedi-kilit.ahk`, `LogitechLedEnginesWrapper.dll`, `third_party/logitech_led/`, kök dizindeki oturum dosyaları. Sonuç: yeni klonda `legacy/` ve SDK yok; CMake eksik SDK'da açık hatayla durur.
- Generator `Visual Studio 18 2026` (Build Tools 18.5, MSVC 14.50, CMake 4.4.4).
- Kod sondan eşleşir (AHK davranışı): öndeki yanlış rakamlar açılışı engellemez.
- Numpad, VK tanınmazsa scan koduyla da tanınır (extended olmayan).
- Ayar aralıkları: `cat_min_simultaneous` 3–32, `cat_min_distinct` 3–64, `cat_window_ms` 50–5000, `idle_lock_min` 0–1440, `alpha` 0–255. Aralık dışı değer varsayılanda kalır.
- `ConfigIssue` değer ve bilinmeyen anahtar adı taşımaz; kod loga geçemez.
- Satır içi yorum desteklenmez.
