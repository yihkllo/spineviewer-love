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
$packageName = Split-Path -Leaf $Destination
$stagingRoot = Join-Path ([IO.Path]::GetTempPath()) ('SpineLoveEX-package-' + [guid]::NewGuid().ToString('N'))
$staging = Join-Path $stagingRoot $packageName
try {
    & (Join-Path $PSScriptRoot 'qt/Deploy-Windows.ps1') -QtRoot $QtRoot -BuildDirectory $BuildDirectory -Destination $staging -FfmpegPath $FfmpegPath
    $userHome = [IO.Path]::GetFullPath($env:USERPROFILE).TrimEnd('\')
    $homeVariants = @($userHome, $userHome.Replace('\', '/'), $userHome.Replace('\', '\\'))
    $problems = foreach ($item in (Get-ChildItem -LiteralPath $staging -Recurse -Force)) {
        $relative = $item.FullName.Substring($staging.Length + 1)
        if ($relative -match '^(favorites|settings|cache)(\\|$)' -or $relative -match '^main\\pro(\\|$)' -or $item.Extension -eq '.ini') { $relative; continue }
        if (!$item.PSIsContainer -and $item.Extension -in @('.conf', '.cfg', '.json', '.txt', '.xml') -and $item.Length -lt 4MB) {
            $text = [IO.File]::ReadAllText($item.FullName)
            foreach ($variant in $homeVariants) { if ($text.IndexOf($variant, [StringComparison]::OrdinalIgnoreCase) -ge 0) { $relative; break } }
        }
    }
    if ($problems) { throw ("The package contains private or user files:`n" + (($problems | Select-Object -First 20) -join "`n")) }
    $archive = $Destination + '.zip'
    $null = New-Item -ItemType Directory -Path (Split-Path -Parent $archive) -Force
    Compress-Archive -LiteralPath $staging -DestinationPath $archive -Force
    Write-Output $archive
}
finally {
    if (Test-Path -LiteralPath $stagingRoot) { Remove-Item -LiteralPath $stagingRoot -Recurse -Force }
}
