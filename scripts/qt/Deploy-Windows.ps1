param(
    [Parameter(Mandatory = $true)][string]$QtRoot,
    [string]$BuildDirectory,
    [string]$Destination,
    [string]$FfmpegPath,
    [string]$MultimediaRoot,
    [string]$ImageFormatsRoot,
    [string]$VisualStudioRoot
)
$ErrorActionPreference = 'Stop'
$projectRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
if (!$BuildDirectory) { $BuildDirectory = Join-Path $projectRoot 'build' }
if (!$Destination) { $Destination = Join-Path $projectRoot 'dist/SpineLoveEX-windows-x64' }
$packageDirectory = [IO.Path]::GetFullPath($Destination)
$launcher = Join-Path $BuildDirectory 'SpineLoveEX.exe'
if (!(Test-Path -LiteralPath $launcher)) { throw 'Build the SpineLoveEX launcher before deploying.' }
if (Test-Path -LiteralPath (Join-Path $packageDirectory 'spinelove_qt.exe')) { throw 'This is a legacy flat package. Use a new destination or organize the existing package first.' }
if (Test-Path -LiteralPath (Join-Path $packageDirectory '_internal')) { throw 'Use the main directory layout or a new destination.' }
$Destination = Join-Path $packageDirectory 'main'
$QtRoot = (Resolve-Path -LiteralPath $QtRoot).Path
if (!$MultimediaRoot) {
    $MultimediaRoot = if (Test-Path -LiteralPath (Join-Path $QtRoot 'bin/Qt6Multimedia.dll')) { $QtRoot } else { Join-Path $projectRoot 'out/deps/qtmultimedia' }
}
$MultimediaRoot = (Resolve-Path -LiteralPath $MultimediaRoot).Path
if (!$ImageFormatsRoot -and (Test-Path -LiteralPath (Join-Path $projectRoot 'out/deps/qtimageformats'))) { $ImageFormatsRoot = Join-Path $projectRoot 'out/deps/qtimageformats' }
if ($ImageFormatsRoot) { $ImageFormatsRoot = (Resolve-Path -LiteralPath $ImageFormatsRoot).Path }
$appDirectory = Join-Path $BuildDirectory 'main/qt'
$executable = Join-Path $appDirectory 'spinelove_qt.exe'
if (!(Test-Path -LiteralPath $executable)) { throw 'Build spinelove_qt before deploying.' }
$Destination = [IO.Path]::GetFullPath($Destination)
if ($Destination.TrimEnd('\','/') -eq [IO.Path]::GetFullPath($appDirectory).TrimEnd('\','/')) { throw 'Deploy to a separate directory.' }
foreach ($protectedRoot in @($QtRoot, $MultimediaRoot, $ImageFormatsRoot, (Join-Path ([IO.Path]::GetFullPath($BuildDirectory)) 'deployment-kit'))) {
    if (!$protectedRoot) { continue }
    $protectedPath = [IO.Path]::GetFullPath($protectedRoot).TrimEnd('\','/')
    if ($Destination.TrimEnd('\','/').Equals($protectedPath, [StringComparison]::OrdinalIgnoreCase) -or $Destination.StartsWith($protectedPath + '\', [StringComparison]::OrdinalIgnoreCase)) {
        throw 'The deployment destination must not overwrite a Qt kit or its local deployment source directory.'
    }
}
$null = New-Item -ItemType Directory -Path $Destination -Force
Copy-Item -LiteralPath (Join-Path $appDirectory 'spinelove_qt.exe') -Destination $Destination -Force
Copy-Item -LiteralPath (Join-Path $appDirectory 'spinelove_sdk.dll') -Destination $Destination -Force
$assetsDirectory = Join-Path $Destination 'ttf'
$null = New-Item -ItemType Directory -Path $assetsDirectory -Force
Copy-Item -LiteralPath (Join-Path $appDirectory 'NotoSansSC-Regular.ttf') -Destination $assetsDirectory -Force
$shaderDirectory = Join-Path $assetsDirectory 'render_d3d11/shaders'
$null = New-Item -ItemType Directory -Path $shaderDirectory -Force
Copy-Item -LiteralPath (Join-Path $projectRoot 'main/render_d3d11/shaders/sprite.hlsl') -Destination $shaderDirectory -Force
$deployer = Join-Path $QtRoot 'bin/windeployqt.exe'
$vsRoot = $VisualStudioRoot
if (!$vsRoot -and $env:VCINSTALLDIR) { $vsRoot = [IO.Path]::GetFullPath((Join-Path $env:VCINSTALLDIR '..')) }
if (!$vsRoot) {
    $vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio/Installer/vswhere.exe'
    if (Test-Path -LiteralPath $vswhere) { $vsRoot = & $vswhere -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath }
}
if (!$vsRoot) {
    foreach ($edition in @('Community','Professional','Enterprise','BuildTools')) {
        $candidate = Join-Path $env:ProgramFiles "Microsoft Visual Studio/2022/$edition"
        if (Test-Path -LiteralPath (Join-Path $candidate 'VC/Redist/MSVC')) { $vsRoot = $candidate; break }
    }
}
if (!$vsRoot) { throw 'The app-local MSVC runtime was not found. Supply -VisualStudioRoot.' }
$redistRoot = Join-Path $vsRoot 'VC/Redist/MSVC'
$crtDirectory = Get-ChildItem -LiteralPath $redistRoot -Directory | Where-Object Name -Match '^\d+\.\d+\.\d+$' | Sort-Object { [version]$_.Name } -Descending |
    ForEach-Object { Get-ChildItem -LiteralPath (Join-Path $_.FullName 'x64') -Directory -Filter 'Microsoft.VC*.CRT' -ErrorAction SilentlyContinue | Sort-Object Name -Descending } |
    Where-Object { Test-Path -LiteralPath (Join-Path $_.FullName 'vcruntime140_1.dll') } | Select-Object -First 1 -ExpandProperty FullName
if (!$crtDirectory) { throw "No complete MSVC x64 runtime exists under $redistRoot" }
$deploymentKit = Join-Path ([IO.Path]::GetFullPath($BuildDirectory)) 'deployment-kit'
$deploymentBin = Join-Path $deploymentKit 'bin'
$null = New-Item -ItemType Directory -Path $deploymentBin -Force
foreach ($sourceRoot in @($QtRoot, $MultimediaRoot)) {
    foreach ($sourceFile in (Get-ChildItem -LiteralPath (Join-Path $sourceRoot 'bin') -Filter 'Qt6*.dll' -File)) {
        $target = Join-Path $deploymentBin $sourceFile.Name
        if (Test-Path -LiteralPath $target) {
            if ((Get-FileHash -LiteralPath $sourceFile.FullName).Hash -eq (Get-FileHash -LiteralPath $target).Hash) { continue }
            Remove-Item -LiteralPath $target -Force
        }
        try { $null = New-Item -ItemType HardLink -Path $target -Target $sourceFile.FullName -ErrorAction Stop }
        catch { Copy-Item -LiteralPath $sourceFile.FullName -Destination $target -Force }
    }
}
$qtpaths = Join-Path $deploymentBin 'qtpaths6.exe'
Copy-Item -LiteralPath (Join-Path $QtRoot 'bin/qtpaths6.exe') -Destination $qtpaths -Force
$baseForward = $QtRoot.Replace('\','/')
$binForward = $deploymentBin.Replace('\','/')
@("[Paths]", "Prefix=$baseForward", "Binaries=$binForward", "Libraries=$baseForward/lib", "Plugins=$baseForward/plugins", "QmlImports=$baseForward/qml") |
    Set-Content -LiteralPath (Join-Path $deploymentBin 'qt.conf') -Encoding utf8
$env:PATH = (Join-Path $MultimediaRoot 'bin') + ';' + (Join-Path $QtRoot 'bin') + ';' + $env:PATH
& $deployer --qtpaths $qtpaths --release --no-compiler-runtime --no-ffmpeg --qmldir (Join-Path $projectRoot 'main/qt') --qmlimport (Join-Path $MultimediaRoot 'qml') --dir $Destination (Join-Path $Destination 'spinelove_qt.exe')
if ($LASTEXITCODE -ne 0) { throw 'Qt deployment failed.' }
foreach ($runtime in (Get-ChildItem -LiteralPath $crtDirectory -Filter '*.dll' -File)) {
    Copy-Item -LiteralPath $runtime.FullName -Destination $Destination -Force
}
$mediaDlls = @('Qt6Multimedia.dll','Qt6MultimediaQuick.dll','avcodec-61.dll','avformat-61.dll','avutil-59.dll','swresample-5.dll','swscale-8.dll')
foreach ($name in $mediaDlls) {
    $sourceFile = Join-Path $MultimediaRoot "bin/$name"
    if (!(Test-Path -LiteralPath $sourceFile)) { throw "Missing matching Qt Multimedia dependency: $sourceFile" }
    Copy-Item -LiteralPath $sourceFile -Destination (Join-Path $Destination $name) -Force
}
$mediaPlugins = Join-Path $Destination 'multimedia'
$null = New-Item -ItemType Directory -Path $mediaPlugins -Force
foreach ($name in @('ffmpegmediaplugin.dll','windowsmediaplugin.dll')) {
    Copy-Item -LiteralPath (Join-Path $MultimediaRoot "plugins/multimedia/$name") -Destination $mediaPlugins -Force
}
$mediaQml = Join-Path $Destination 'qml/QtMultimedia'
$null = New-Item -ItemType Directory -Path $mediaQml -Force
foreach ($name in @('qmldir','plugins.qmltypes','Video.qml','quickmultimediaplugin.dll')) {
    Copy-Item -LiteralPath (Join-Path $MultimediaRoot "qml/QtMultimedia/$name") -Destination $mediaQml -Force
}
if ($ImageFormatsRoot) {
    $imagePlugins = Join-Path $Destination 'imageformats'
    $null = New-Item -ItemType Directory -Path $imagePlugins -Force
    foreach ($plugin in (Get-ChildItem -LiteralPath (Join-Path $ImageFormatsRoot 'plugins/imageformats') -Filter '*.dll' -File)) {
        if (!$plugin.BaseName.EndsWith('d')) { Copy-Item -LiteralPath $plugin.FullName -Destination $imagePlugins -Force }
    }
}
if ($FfmpegPath) {
    $FfmpegPath = (Resolve-Path -LiteralPath $FfmpegPath).Path
    $targetFfmpeg = Join-Path $Destination 'ffmpeg.exe'
    if (!$FfmpegPath.Equals([IO.Path]::GetFullPath($targetFfmpeg), [StringComparison]::OrdinalIgnoreCase)) {
        Copy-Item -LiteralPath $FfmpegPath -Destination $targetFfmpeg -Force
    }
}
$licenseDirectory = Join-Path $Destination 'licenses'
$null = New-Item -ItemType Directory -Path $licenseDirectory -Force
Copy-Item -LiteralPath (Join-Path $projectRoot 'LICENSE') -Destination (Join-Path $licenseDirectory 'PROJECT_LICENSE') -Force
Copy-Item -LiteralPath (Join-Path $projectRoot 'third_party/live2d/CORE_LICENSE.md') -Destination $licenseDirectory -Force
Copy-Item -LiteralPath (Join-Path $projectRoot 'third_party/live2d/FRAMEWORK_LICENSE.md') -Destination $licenseDirectory -Force
Copy-Item -LiteralPath (Join-Path $projectRoot 'third_party/nlohmann/LICENSE.MIT') -Destination (Join-Path $licenseDirectory 'NLOHMANN_JSON_LICENSE.MIT') -Force
Get-ChildItem -LiteralPath (Join-Path $projectRoot 'docs/licenses') -File | Copy-Item -Destination $licenseDirectory -Force
$multimediaSbom = Join-Path $MultimediaRoot 'sbom/qtmultimedia-6.8.3.spdx.json'
if (Test-Path -LiteralPath $multimediaSbom) { Copy-Item -LiteralPath $multimediaSbom -Destination $licenseDirectory -Force }
Copy-Item -LiteralPath $launcher -Destination (Join-Path $packageDirectory 'SpineLoveEX.exe') -Force
Write-Output "Portable package: $packageDirectory"
Write-Output 'Media playback uses the bundled Qt Multimedia codecs. Video export requires ffmpeg.exe beside the app or on PATH.'
