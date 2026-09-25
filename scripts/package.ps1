param(
    [string]$QtRoot = 'C:/Qt/6.8.3/msvc2022_64',
    [string]$BuildDirectory,
    [string]$Destination,
    [string]$FfmpegPath
)
$ErrorActionPreference = 'Stop'
$projectRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
if (!$BuildDirectory) { $BuildDirectory = Join-Path $projectRoot 'build' }
if (!$Destination) { $Destination = Join-Path $projectRoot 'dist/SpineLoveEX-windows-x64' }
$Destination = [IO.Path]::GetFullPath($Destination)
& (Join-Path $PSScriptRoot 'qt/Deploy-Windows.ps1') -QtRoot $QtRoot -BuildDirectory $BuildDirectory -Destination $Destination -FfmpegPath $FfmpegPath
$archive = $Destination + '.zip'
Compress-Archive -LiteralPath $Destination -DestinationPath $archive -Force
Write-Output $archive
