param(
    [string]$QtRoot = 'C:/Qt/6.8.3/msvc2022_64',
    [string]$BuildDirectory,
    [string]$ShaderToolsRoot,
    [ValidateRange(1,64)][int]$Parallel = 8,
    [switch]$Deploy,
    [switch]$Clean
)
$ErrorActionPreference = 'Stop'
$projectRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
if (!$BuildDirectory) { $BuildDirectory = Join-Path $projectRoot 'build' }
if (!$ShaderToolsRoot) {
    $localShaderTools = Join-Path $projectRoot 'out/deps/qtshadertools'
    if (Test-Path -LiteralPath $localShaderTools) { $ShaderToolsRoot = $localShaderTools }
}
& (Join-Path $PSScriptRoot 'qt/Build-Windows.ps1') -QtRoot $QtRoot -BuildDirectory $BuildDirectory -ShaderToolsRoot $ShaderToolsRoot -Parallel $Parallel -Deploy:$Deploy -Clean:$Clean
