---
name: oturum-logu
description: cat-lock-cpp (eski adı kedi-kilit) oturumlarında yapılanları E:\Documents\Claude-Logs klasörüne oturum başına bir .md dosyası olarak logla.
---

# Oturum logu

Amaç: Her oturumun yerel, okunur kaydı. Bir oturum = bir dosya.

## Konum ve ad

- Klasör: `E:\Documents\Claude-Logs\` (erişim `.claude/settings.json` → `additionalDirectories`)
- Dosya: `YYYY-MM-DD_HHMM_<kisa-konu>.md` (yerel saat; konu küçük harf, tireli, Türkçe karakter yok)
- Oturum konusu değişirse dosya adını değiştirme; içine yeni bölüm başlığı ekle.

## Saat

Saati tahmin etme, komutla al:

```powershell
(Get-Date).ToString('ddd dd MMM yyyy · HH:mm', [Globalization.CultureInfo]'tr-TR') + ' (TSİ)'
```

## Dosya formatı

~~~
# Oturum Logu — YYYY-MM-DD (HH:MM–HH:MM, UTC+3)
Proje: kedi-kilit · Cihaz: <bilgisayar adı>

| Saat | İstek | İşlem | Araç / Uygulama | Sonuç |
|---|---|---|---|---|

## Varsayımlar
- HH:MM — <neyi varsaydım> — sebep: <Atla seçildi / manuel cevap yok / belirsiz istek>
(yoksa: "Bu oturumda varsayım yok.")

## Değişiklikler
- Dosyalar: ...
~~~

## Detay: orta

Her anlamlı adım için bir satır: saat, kısa istek, yapılan işlem, araç, sonuç. Değişen dosyayı adıyla yaz. Script tam metni yazılmaz; ne yaptığı bir cümleyle yazılır.

Kilit kodu, parola, token ve benzeri gizli değerler asla loglanmaz. `cat-lock-cpp.ini` ve eski `kedi-kilit.ini` okunmaz.

## Varsayımlar

Kullanıcı soru kartında "Atla" seçerse, cevap vermezse ya da bir şeyi varsaymak gerekirse: varsayımı bir sonraki mesajda açıkça belirt, `## Varsayımlar` altına saat ve sebebiyle ekle, ilgili satırın Sonuç sütununa "(varsayım)" yaz.

## Yazma zamanı

Adım adım yazma. Satırları biriktir, yalnız şu anlarda doğrudan dosyaya yaz:

1. Bir iş bittiğinde (sonuç raporlanmadan hemen önce)
2. Soru kartı sormadan hemen önce
3. Oturum sonunda (`## Değişiklikler` ve bitiş saatiyle)

Saatler işlemin gerçek zamanını gösterir, yazma anını değil.

## Raporlama

Loglama sessiz yapılır. Oturum sonunda tek satır: log dosyasının adı ve yeri.
