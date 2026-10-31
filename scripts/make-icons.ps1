# Pati ikonlarını üretir: src/win/res/paw.ico, paw-banned.ico
# Yalnız ikon tasarımı değişince elle çalıştırılır; build'in parçası değil.
# Kullanım: pwsh scripts/make-icons.ps1 [-PreviewDir <klasör>]
param([string]$PreviewDir)

Add-Type -AssemblyName System.Drawing
$ErrorActionPreference = 'Stop'

$sizes = 16, 20, 24, 32, 40, 48, 64, 256
$outDir = Join-Path $PSScriptRoot '..\src\win\res'
New-Item -ItemType Directory -Force $outDir | Out-Null

function New-PawBitmap([int]$size, [bool]$banned) {
    $bmp = [System.Drawing.Bitmap]::new($size, $size, [System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
    $g = [System.Drawing.Graphics]::FromImage($bmp)
    $g.SmoothingMode = [System.Drawing.Drawing2D.SmoothingMode]::AntiAlias
    $g.ScaleTransform($size / 256.0, $size / 256.0)

    $fill = [System.Drawing.SolidBrush]::new([System.Drawing.Color]::FromArgb(255, 245, 158, 66))
    $outlineWidth = if ($size -ge 32) { 8.0 } else { 0.0 }
    $outline = [System.Drawing.Pen]::new([System.Drawing.Color]::FromArgb(255, 74, 44, 16), $outlineWidth)

    $ellipse = {
        param($cx, $cy, $rx, $ry, $angle)
        $state = $g.Save()
        $g.TranslateTransform($cx, $cy)
        $g.RotateTransform($angle)
        $g.FillEllipse($fill, -$rx, -$ry, 2 * $rx, 2 * $ry)
        if ($outlineWidth -gt 0) { $g.DrawEllipse($outline, -$rx, -$ry, 2 * $rx, 2 * $ry) }
        $g.Restore($state)
    }

    & $ellipse 128 176 74 58 0     # ana yastık
    & $ellipse 50 112 28 38 -24    # parmaklar
    & $ellipse 102 62 30 42 -7
    & $ellipse 154 62 30 42 7
    & $ellipse 206 112 28 38 24

    if ($banned) {
        # Küçük boyutlarda çizgi en az 2 piksel kalsın.
        $width = [Math]::Max(30.0, 2.0 * 256.0 / $size)
        $pen = [System.Drawing.Pen]::new([System.Drawing.Color]::FromArgb(255, 225, 29, 46), $width)
        $radius = 128.0 - $width / 2.0 - 2.0
        $g.DrawEllipse($pen, 128 - $radius, 128 - $radius, 2 * $radius, 2 * $radius)
        $d = $radius * 0.7071
        $g.DrawLine($pen, 128 - $d, 128 - $d, 128 + $d, 128 + $d)
        $pen.Dispose()
    }

    $outline.Dispose(); $fill.Dispose(); $g.Dispose()
    return $bmp
}

function Write-Ico([string]$path, [bool]$banned) {
    $images = foreach ($size in $sizes) {
        $bmp = New-PawBitmap $size $banned
        $stream = [System.IO.MemoryStream]::new()
        $bmp.Save($stream, [System.Drawing.Imaging.ImageFormat]::Png)
        $bmp.Dispose()
        [pscustomobject]@{ Size = $size; Data = $stream.ToArray() }
    }

    $file = [System.IO.File]::Create($path)
    $w = [System.IO.BinaryWriter]::new($file)
    $w.Write([uint16]0); $w.Write([uint16]1); $w.Write([uint16]$images.Count)
    $offset = 6 + 16 * $images.Count
    foreach ($image in $images) {
        $dim = if ($image.Size -ge 256) { 0 } else { $image.Size }
        $w.Write([byte]$dim); $w.Write([byte]$dim); $w.Write([byte]0); $w.Write([byte]0)
        $w.Write([uint16]1); $w.Write([uint16]32)
        $w.Write([uint32]$image.Data.Length); $w.Write([uint32]$offset)
        $offset += $image.Data.Length
    }
    foreach ($image in $images) { $w.Write($image.Data) }
    $w.Dispose()
}

Write-Ico (Join-Path $outDir 'paw.ico') $false
Write-Ico (Join-Path $outDir 'paw-banned.ico') $true

if ($PreviewDir) {
    # Her iki ikon, dört boyut, açık ve koyu zemin; 4 kat büyütülmüş.
    $show = 16, 24, 32, 64
    $sheet = [System.Drawing.Bitmap]::new(760, 560)
    $g = [System.Drawing.Graphics]::FromImage($sheet)
    $g.InterpolationMode = [System.Drawing.Drawing2D.InterpolationMode]::NearestNeighbor
    $g.Clear([System.Drawing.Color]::White)
    $g.FillRectangle([System.Drawing.Brushes]::Black, 380, 0, 380, 560)
    foreach ($col in 0, 1) {
        $y = 10
        foreach ($banned in $false, $true) {
            $x = 10 + 380 * $col
            foreach ($size in $show) {
                $bmp = New-PawBitmap $size $banned
                $g.DrawImage($bmp, $x, $y, $size * 4, $size * 4)
                $x += [Math]::Min($size * 4 + 8, 120)
                $bmp.Dispose()
            }
            $y += 275
        }
    }
    $g.Dispose()
    $sheet.Save((Join-Path $PreviewDir 'icons.png'), [System.Drawing.Imaging.ImageFormat]::Png)
    $sheet.Dispose()
}
