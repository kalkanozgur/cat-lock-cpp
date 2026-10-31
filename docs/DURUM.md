# Durum

## Tamamlanan

- **Adım 0 — İlk kurulum (31.10.2026):** iskelet Debug ve Release'te 0 uyarıyla derleniyor, `kk_core_tests` 1/1. Kurulum notları: `docs/kurulum.md`.

## Sıradaki

- **Adım 1:** core: `config`, `key_map`, `code_matcher` + testler. Bitti sayılır: ctest yeşil.

## Açık sorular

- `kedi-kilit.exe` iskeleti hiç çalıştırılmadı (yasak); ilk çalıştırma testi Adım 2'de kullanıcıda.
- Güncel AHK (`E:\Downloads\LED_SDK_9.00\kedi-kilit.ahk`, rakam kodlu) ile `legacy/kedi-kilit.ahk` (eski, harf parolalı) farklı. CLAUDE.md'nin "çalışan davranış örneği" bu klasördeki eski sürüm değil; referans CPP-PLAN.md.
- Aynı SDK klasöründeki imzasız `kedi-kilit.exe` (30.10.2026) incelenmedi ve çalıştırılmadı.

## Varsayımlar ve kararlar

- Repo: `kalkanozgur/cat-lock-cpp` (private), kullanıcı kararı. Push yalnız kullanıcı isteyince.
- Repoda olmayanlar (`.gitignore`), kullanıcı kararı: `kedi-kilit.ahk`, `LogitechLedEnginesWrapper.dll`, `third_party/logitech_led/`, kök dizindeki oturum dosyaları. Sonuç: yeni klonda `legacy/` ve SDK yok; CMake eksik SDK'da açık hatayla durur.
- Generator `Visual Studio 18 2026` (Build Tools 18.5, MSVC 14.50, CMake 4.4.4).
- `kk_core`'un Adım 1'e kadar tek içeriği `src/core/version.*`; test olsun diye eklendi, Adım 1'de kaldırılabilir.
