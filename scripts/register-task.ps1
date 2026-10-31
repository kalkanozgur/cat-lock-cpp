#Requires -RunAsAdministrator
<#
.SYNOPSIS
    KediKilit'i Program Files'a kurar ve oturum açılışında yüksek yetkiyle başlatır.
.PARAMETER Remove
    Görevi ve kurulu dosyaları kaldırır. Ayar ve log (%LOCALAPPDATA%\KediKilit) kalır.
.NOTES
    Yönetici PowerShell'de:
      powershell -ExecutionPolicy Bypass -File .\scripts\register-task.ps1
      powershell -ExecutionPolicy Bypass -File .\scripts\register-task.ps1 -Remove
#>
[CmdletBinding()]
param(
    [switch]$Remove
)

$ErrorActionPreference = 'Stop'

$TaskName = 'KediKilit'
$InstallDir = Join-Path $env:ProgramFiles 'KediKilit'
$InstalledExe = Join-Path $InstallDir 'kedi-kilit.exe'
$ReleaseExe = Join-Path $PSScriptRoot '..\build\Release\kedi-kilit.exe'

function Stop-Installed {
    $task = Get-ScheduledTask -TaskName $TaskName -ErrorAction SilentlyContinue
    if ($task) {
        Stop-ScheduledTask -TaskName $TaskName -ErrorAction SilentlyContinue
    }
    # Yalnız kurulu kopya: build klasöründen çalışan Debug'a dokunulmaz.
    Get-Process -Name 'kedi-kilit' -ErrorAction SilentlyContinue |
        Where-Object { $_.Path -eq $InstalledExe } |
        ForEach-Object {
            Stop-Process -Id $_.Id -Force
            $_.WaitForExit(5000) | Out-Null
        }
}

if ($Remove) {
    Stop-Installed
    if (Get-ScheduledTask -TaskName $TaskName -ErrorAction SilentlyContinue) {
        Unregister-ScheduledTask -TaskName $TaskName -Confirm:$false
    }
    if (Test-Path -LiteralPath $InstallDir) {
        Remove-Item -LiteralPath $InstallDir -Recurse -Force
    }
    Write-Host 'KediKilit kaldırıldı. Ayar ve log %LOCALAPPDATA%\KediKilit içinde kaldı.'
    return
}

if (-not (Test-Path -LiteralPath $ReleaseExe)) {
    throw "Release exe yok: $ReleaseExe. Önce: cmake --build --preset release"
}
$other = Get-Process -Name 'kedi-kilit' -ErrorAction SilentlyContinue |
    Where-Object { $_.Path -ne $InstalledExe }
if ($other) {
    throw 'Build klasöründen çalışan bir kopya var. Tepsiden Çıkış yap, sonra yeniden çalıştır.'
}

Stop-Installed
New-Item -ItemType Directory -Force -Path $InstallDir | Out-Null
Copy-Item -LiteralPath $ReleaseExe -Destination $InstalledExe -Force

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

Write-Host "KediKilit kuruldu: $InstalledExe"
Write-Host "Görev: $TaskName, kullanıcı: $user, oturum açılışında, en yüksek ayrıcalıklarla. Şimdi başlatıldı."
