param(
    [string]$EngineRoot = 'C:\Program Files\Epic Games\UE_5.6'
)
$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path $PSScriptRoot
$projectFile = Join-Path $projectRoot 'CH4_2.uproject'
$editor = Join-Path $EngineRoot 'Engine\Binaries\Win64\UnrealEditor-Cmd.exe'
$logDirectory = Join-Path $projectRoot 'Saved\Validation'
New-Item -ItemType Directory -Force $logDirectory | Out-Null
$runId = Get-Date -Format 'yyyyMMdd-HHmmss'
$hostLog = Join-Path $logDirectory "vehicle-host-$runId.log"
$clientLog = Join-Path $logDirectory "vehicle-client-$runId.log"
$serverProcess = $null
$clientProcess = $null
try {
    $serverProcess = Start-Process -FilePath $editor -WindowStyle Hidden -PassThru -ArgumentList @(
        ('"{0}"' -f $projectFile), '/Game/VehicleTemplate/Maps/VehicleBasic?listen', '-game', '-port=17877',
        '-unattended', '-nop4', '-NullRHI', '-nosound', ('-abslog="{0}"' -f $hostLog)
    )
    $deadline = (Get-Date).AddSeconds(45)
    do {
        Start-Sleep -Milliseconds 500
        $hostText = if (Test-Path $hostLog) { Get-Content $hostLog -Raw } else { '' }
        if ($serverProcess.HasExited) { throw "Host exited early. See $hostLog" }
    } until ($hostText.Contains('GameNetDriver') -or (Get-Date) -gt $deadline)
    if (-not $hostText.Contains('GameNetDriver')) { throw "Host listen timeout. See $hostLog" }
    $clientProcess = Start-Process -FilePath $editor -WindowStyle Hidden -PassThru -ArgumentList @(
        ('"{0}"' -f $projectFile), '127.0.0.1:17877', '-game', '-unattended', '-nop4', '-NullRHI', '-nosound',
        ('-abslog="{0}"' -f $clientLog)
    )
    $deadline = (Get-Date).AddSeconds(45)
    do {
        Start-Sleep -Milliseconds 500
        $hostText = Get-Content $hostLog -Raw
        $clientText = if (Test-Path $clientLog) { Get-Content $clientLog -Raw } else { '' }
        if ($serverProcess.HasExited -or $clientProcess.HasExited) { throw 'Engine process exited early' }
        $possessions = [regex]::Matches($hostText, 'Vehicle possessed: Controller=.*? Location=([^\r\n]+)')
        $clientReady = $clientText.Contains('Vehicle reference replicated:') -and $clientText.Contains('Vehicle HUD created for local controller')
    } until (($possessions.Count -ge 2 -and $clientReady) -or (Get-Date) -gt $deadline)
    if ($possessions.Count -lt 2 -or -not $clientReady) { throw "Network smoke timeout: $hostLog ; $clientLog" }
    if ($possessions[0].Groups[1].Value -eq $possessions[1].Groups[1].Value) { throw 'Both vehicles spawned at the same location' }
    if ($hostText.Contains('Fatal error:') -or $clientText.Contains('Fatal error:')) { throw 'Fatal runtime error' }
    Write-Output "PASS: listen host and client possessed separate vehicles; client received Pawn reference and created local HUD."
    Write-Output "Host log: $hostLog"
    Write-Output "Client log: $clientLog"
    Write-Output 'Not verified: keyboard driving, visual HUD, physics convergence, occupied-all-starts branch.'
} finally {
    foreach ($process in @($clientProcess, $serverProcess)) {
        if ($null -ne $process -and -not $process.HasExited) { Stop-Process -Id $process.Id }
    }
}
