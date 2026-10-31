#Requires -RunAsAdministrator
<#
.SYNOPSIS
    cat-lock-cpp'yi Program Files'a kurar ve oturum açılışında yüksek yetkiyle başlatır.
    Eski KediKilit kurulumu varsa kaldırır.
.PARAMETER Remove
    Görevi ve kurulu dosyaları kaldırır. Ayar ve log (%LOCALAPPDATA%\CatLockCpp) kalır.
.NOTES
    Yönetici PowerShell'de:
      powershell -ExecutionPolicy Bypass -File .\register-task.ps1
      powershell -ExecutionPolicy Bypass -File .\register-task.ps1 -Remove
    Exe aranan yerler: betikle aynı klasör (release zip), ..\build\Release (repo).
#>
[CmdletBinding()]
param(
    [switch]$Remove
)

$ErrorActionPreference = 'Stop'

$TaskName = 'CatLockCpp'
$InstallDir = Join-Path $env:ProgramFiles 'CatLockCpp'
$InstalledExe = Join-Path $InstallDir 'cat-lock-cpp.exe'

$LegacyTaskName = 'KediKilit'
$LegacyInstallDir = Join-Path $env:ProgramFiles 'KediKilit'
$LegacyExe = Join-Path $LegacyInstallDir 'kedi-kilit.exe'

$SourceExe = Join-Path $PSScriptRoot 'cat-lock-cpp.exe'
if (-not (Test-Path -LiteralPath $SourceExe)) {
    $SourceExe = Join-Path $PSScriptRoot '..\build\Release\cat-lock-cpp.exe'
}

function Stop-Installed([string]$Name, [string]$Exe) {
    if (Get-ScheduledTask -TaskName $Name -ErrorAction SilentlyContinue) {
        Stop-ScheduledTask -TaskName $Name -ErrorAction SilentlyContinue
    }
    # Yalnız kurulu kopya: build klasöründen çalışan Debug'a dokunulmaz.
    Get-Process -Name ([IO.Path]::GetFileNameWithoutExtension($Exe)) -ErrorAction SilentlyContinue |
        Where-Object { $_.Path -eq $Exe } |
        ForEach-Object {
            Stop-Process -Id $_.Id -Force
            $_.WaitForExit(5000) | Out-Null
        }
}

function Remove-Installed([string]$Name, [string]$Dir, [string]$Exe) {
    Stop-Installed $Name $Exe
    if (Get-ScheduledTask -TaskName $Name -ErrorAction SilentlyContinue) {
        Unregister-ScheduledTask -TaskName $Name -Confirm:$false
    }
    if (Test-Path -LiteralPath $Dir) {
        Remove-Item -LiteralPath $Dir -Recurse -Force
    }
}

if ($Remove) {
    & (Join-Path $PSScriptRoot 'uninstall.ps1')
    return
}

if (-not (Test-Path -LiteralPath $SourceExe)) {
    throw "Exe yok: $SourceExe. Repoda önce: cmake --build --preset release"
}
$other = Get-Process -Name 'cat-lock-cpp', 'kedi-kilit' -ErrorAction SilentlyContinue |
    Where-Object { $_.Path -ne $InstalledExe -and $_.Path -ne $LegacyExe }
if ($other) {
    throw 'Build klasöründen çalışan bir kopya var. Tepsiden Çıkış yap, sonra yeniden çalıştır.'
}

# Eski sürüm durmadan yenisi başlamasın: ayar taşınırken ini ve log serbest olmalı.
Remove-Installed $LegacyTaskName $LegacyInstallDir $LegacyExe

Stop-Installed $TaskName $InstalledExe
New-Item -ItemType Directory -Force -Path $InstallDir | Out-Null
Copy-Item -LiteralPath $SourceExe -Destination $InstalledExe -Force

$user = [Security.Principal.WindowsIdentity]::GetCurrent().Name
$action = New-ScheduledTaskAction -Execute $InstalledExe -WorkingDirectory $InstallDir
$trigger = New-ScheduledTaskTrigger -AtLogOn -User $user
$principal = New-ScheduledTaskPrincipal -UserId $user -LogonType Interactive -RunLevel Highest
# Priority 4: varsayılan 7 düşük öncelik, hook gecikmesi riski. Süre sınırı yok.
$settings = New-ScheduledTaskSettingsSet -AllowStartIfOnBatteries -DontStopIfGoingOnBatteries `
    -ExecutionTimeLimit ([TimeSpan]::Zero) -MultipleInstances IgnoreNew -Priority 4

Register-ScheduledTask -TaskName $TaskName -Action $action -Trigger $trigger `
    -Principal $principal -Settings $settings -Force | Out-Null
Start-ScheduledTask -TaskName $TaskName

Write-Host "cat-lock-cpp kuruldu: $InstalledExe"
Write-Host "Görev: $TaskName, kullanıcı: $user, oturum açılışında, en yüksek ayrıcalıklarla. Şimdi başlatıldı."
