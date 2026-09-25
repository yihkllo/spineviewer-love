param(
    [Parameter(Mandatory = $true)][string]$QtRoot,
    [string]$ShaderToolsRoot,
    [string]$MultimediaRoot,
    [string]$ImageFormatsRoot,
    [string]$VisualStudioRoot,
    [string]$BuildDirectory,
    [string]$SourceDirectory,
    [string[]]$CMakeArguments = @(),
    [ValidateRange(1,64)][int]$Parallel = 8,
    [switch]$Deploy,
    [switch]$Clean
)
$ErrorActionPreference = 'Stop'
$projectRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
if (!$BuildDirectory) { $BuildDirectory = Join-Path $projectRoot 'build' }
$QtRoot = (Resolve-Path -LiteralPath $QtRoot).Path
$vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio/Installer/vswhere.exe'
$vsRoot = $VisualStudioRoot
if (!$vsRoot -and (Test-Path -LiteralPath $vswhere)) {
    $vsRoot = & $vswhere -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
}
if (!$vsRoot) {
    foreach ($edition in @('Community','Professional','Enterprise','BuildTools')) {
        $candidate = Join-Path $env:ProgramFiles "Microsoft Visual Studio/2022/$edition"
        if (Test-Path -LiteralPath (Join-Path $candidate 'VC/Auxiliary/Build/vcvars64.bat')) { $vsRoot = $candidate; break }
    }
}
if (!$vsRoot) { throw 'An MSVC x64 toolchain was not found. Supply -VisualStudioRoot for an offline installation.' }
$vcvars = Join-Path $vsRoot 'VC/Auxiliary/Build/vcvars64.bat'
$environment = & $env:ComSpec /d /c ('"{0}" >nul && set' -f $vcvars)
if ($LASTEXITCODE -ne 0) { throw 'Could not initialize MSVC.' }
foreach ($line in $environment) {
    if ($line -match '^([^=]+)=(.*)$') { [Environment]::SetEnvironmentVariable($matches[1], $matches[2], 'Process') }
}
$cmake = Join-Path $vsRoot 'Common7/IDE/CommonExtensions/Microsoft/CMake/CMake/bin/cmake.exe'
if (!(Test-Path -LiteralPath $cmake)) { $cmake = (Get-Command cmake -ErrorAction Stop).Source }
$env:PATH = (Join-Path $QtRoot 'bin') + ';' + $env:PATH
$env:VSLANG = '1033'
$BuildDirectory = [IO.Path]::GetFullPath($BuildDirectory)
$probeDirectory = Join-Path $BuildDirectory 'compiler-dependency-probe'
$null = New-Item -ItemType Directory -Path $probeDirectory -Force
$probeSource = Join-Path $probeDirectory 'include-prefix.cpp'
[IO.File]::WriteAllText($probeSource, "#include <stddef.h>`nint slDependencyProbe;`n", [Text.UTF8Encoding]::new($false))
$probeObject = Join-Path $probeDirectory 'include-prefix.obj'
$includeOutput = & cl.exe /nologo /showIncludes /c /TP /utf-8 "/Fo$probeObject" $probeSource 2>&1
if ($LASTEXITCODE -ne 0) { throw 'Could not detect the MSVC include dependency prefix.' }
$includePrefix = $null
foreach ($line in $includeOutput) {
    $prefixMatch = [regex]::Match($line.ToString(), '^(.*?)[A-Za-z]:[\\/].*[\\/]stddef\.h\s*$')
    if ($prefixMatch.Success) { $includePrefix = $prefixMatch.Groups[1].Value; break }
}
if ([string]::IsNullOrEmpty($includePrefix)) { throw 'MSVC include output was not recognized; refusing to build without header dependencies.' }
$prefixes = @($QtRoot)
$sourceRoot = if ($SourceDirectory) { (Resolve-Path -LiteralPath $SourceDirectory).Path } else { $projectRoot }
$configure = @('-S', $sourceRoot, '-B', $BuildDirectory, '-G', 'Ninja',
    '-DCMAKE_BUILD_TYPE=Release', "-DSPINELOVE_MSVC_INCLUDE_PREFIX=$includePrefix")
if (!$SourceDirectory) { $configure += @('-DSPINELOVE_BUILD_QT_APP=ON', '-DSPINELOVE_BUILD_RUNTIME_TESTS=ON') }
$ninja = Join-Path $vsRoot 'Common7/IDE/CommonExtensions/Microsoft/CMake/Ninja/ninja.exe'
if (Test-Path -LiteralPath $ninja) { $configure += "-DCMAKE_MAKE_PROGRAM=$ninja" }
if ($ShaderToolsRoot) {
    $ShaderToolsRoot = (Resolve-Path -LiteralPath $ShaderToolsRoot).Path
    $env:PATH = (Join-Path $ShaderToolsRoot 'bin') + ';' + $env:PATH
    $prefixes += $ShaderToolsRoot
    $configure += "-DQt6ShaderTools_DIR=$ShaderToolsRoot/lib/cmake/Qt6ShaderTools"
    $configure += "-DQt6ShaderToolsTools_DIR=$ShaderToolsRoot/lib/cmake/Qt6ShaderToolsTools"
}
if (!$MultimediaRoot -and !(Test-Path -LiteralPath (Join-Path $QtRoot 'lib/cmake/Qt6Multimedia/Qt6MultimediaConfig.cmake'))) {
    $MultimediaRoot = Join-Path $projectRoot 'out/deps/qtmultimedia'
}
if ($MultimediaRoot) {
    $MultimediaRoot = (Resolve-Path -LiteralPath $MultimediaRoot).Path
    $prefixes += $MultimediaRoot
    $env:PATH = (Join-Path $MultimediaRoot 'bin') + ';' + $env:PATH
    $env:QT_PLUGIN_PATH = (Join-Path $MultimediaRoot 'plugins') + ';' + $env:QT_PLUGIN_PATH
    $env:QML_IMPORT_PATH = (Join-Path $MultimediaRoot 'qml') + ';' + $env:QML_IMPORT_PATH
    $configure += "-DQt6Multimedia_DIR=$MultimediaRoot/lib/cmake/Qt6Multimedia"
}
if (!$ImageFormatsRoot -and (Test-Path -LiteralPath (Join-Path $projectRoot 'out/deps/qtimageformats'))) {
    $ImageFormatsRoot = Join-Path $projectRoot 'out/deps/qtimageformats'
}
if ($ImageFormatsRoot) {
    $ImageFormatsRoot = (Resolve-Path -LiteralPath $ImageFormatsRoot).Path
    $env:QT_PLUGIN_PATH = (Join-Path $ImageFormatsRoot 'plugins') + ';' + $env:QT_PLUGIN_PATH
}
$configure += $CMakeArguments
$configure += "-DCMAKE_PREFIX_PATH=$($prefixes -join ';')"
& $cmake @configure
if ($LASTEXITCODE -ne 0) { throw 'Qt CMake configuration failed.' }
$buildArguments = @('--build', $BuildDirectory, '--parallel', $Parallel)
if ($Clean) { $buildArguments += '--clean-first' }
& $cmake @buildArguments
if ($LASTEXITCODE -ne 0) { throw 'Qt build failed.' }
if ($Deploy -and !$SourceDirectory) { & (Join-Path $PSScriptRoot 'Deploy-Windows.ps1') -QtRoot $QtRoot -MultimediaRoot $MultimediaRoot -ImageFormatsRoot $ImageFormatsRoot -VisualStudioRoot $vsRoot -BuildDirectory $BuildDirectory }
