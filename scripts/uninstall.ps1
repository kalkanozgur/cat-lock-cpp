#Requires -RunAsAdministrator
<#
.SYNOPSIS
    cat-lock-cpp'yi ve eski KediKilit kurulumunu bu bilgisayardan kaldırır.
.PARAMETER Purge
    Ayar ve logu da siler. Kilit kodu bu ayar dosyasındadır.
.NOTES
    Çift tıkla: uninstall.cmd (ayar ve log kalır).
    Yönetici PowerShell'de:
      powershell -ExecutionPolicy Bypass -File .\uninstall.ps1
      powershell -ExecutionPolicy Bypass -File .\uninstall.ps1 -Purge
#>
[CmdletBinding()]
param(
    [switch]$Purge
)

$ErrorActionPreference = 'Stop'

$Installs = @(
    @{ Task = 'CatLockCpp'; Dir = (Join-Path $env:ProgramFiles 'CatLockCpp'); Process = 'cat-lock-cpp'
       Data = (Join-Path $env:LOCALAPPDATA 'CatLockCpp') },
    @{ Task = 'KediKilit'; Dir = (Join-Path $env:ProgramFiles 'KediKilit'); Process = 'kedi-kilit'
       Data = (Join-Path $env:LOCALAPPDATA 'KediKilit') }
)

foreach ($install in $Installs) {
    $exe = Join-Path $install.Dir "$($install.Process).exe"
    $task = Get-ScheduledTask -TaskName $install.Task -ErrorAction SilentlyContinue
    if ($task) {
        Stop-ScheduledTask -TaskName $install.Task -ErrorAction SilentlyContinue
    }
    # Yalnız kurulu kopya: build klasöründen çalışan kopyaya dokunulmaz.
    Get-Process -Name $install.Process -ErrorAction SilentlyContinue |
        Where-Object { $_.Path -eq $exe } |
        ForEach-Object {
            Stop-Process -Id $_.Id -Force
            $_.WaitForExit(5000) | Out-Null
        }
    if ($task) {
        Unregister-ScheduledTask -TaskName $install.Task -Confirm:$false
        Write-Host "Görev kaldırıldı: $($install.Task)"
    }
    if (Test-Path -LiteralPath $install.Dir) {
        Remove-Item -LiteralPath $install.Dir -Recurse -Force
        Write-Host "Klasör silindi: $($install.Dir)"
    }
    if ($Purge -and (Test-Path -LiteralPath $install.Data)) {
        Remove-Item -LiteralPath $install.Data -Recurse -Force
        Write-Host "Ayar ve log silindi: $($install.Data)"
    }
}

if ($Purge) {
    Write-Host 'Kaldırma tamamlandı.'
} else {
    Write-Host 'Kaldırma tamamlandı. Ayar ve log %LOCALAPPDATA% altında duruyor; yeni kurulum onları kullanır.'
}
