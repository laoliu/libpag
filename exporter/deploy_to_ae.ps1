# 部署 PAGExporter 到 After Effects
# AE 插件必须直接在 Plug-ins 目录下，不能在子目录中

# 设置路径
$buildPath = "H:\work\python\libpag\exporter\build\Release"
$aePath = "C:\Program Files\Adobe\Adobe After Effects 2024\Support Files\Plug-ins"  # 修改为你的 AE 版本

# 直接复制主插件文件到 Plug-ins 目录
Copy-Item -Path (Join-Path $buildPath "PAGExporter.aex") -Destination $aePath -Force
Write-Host "Copied PAGExporter.aex to Plug-ins directory"

# 复制 Qt DLLs 到 Plug-ins 目录（与 .aex 同级）
$qtDlls = @(
    "Qt6Core.dll",
    "Qt6Gui.dll", 
    "Qt6Qml.dll",
    "Qt6Quick.dll",
    "Qt6QuickControls2.dll",
    "Qt6Network.dll",
    "Qt6Widgets.dll"
)

$qtDllPath = Join-Path $buildPath "QTDll"
foreach ($dll in $qtDlls) {
    $sourceDll = Join-Path $qtDllPath $dll
    if (Test-Path $sourceDll) {
        Copy-Item -Path $sourceDll -Destination $aePath -Force
        Write-Host "Copied $dll to Plug-ins directory"
    } else {
        Write-Host "Warning: $dll not found at $sourceDll" -ForegroundColor Yellow
    }
}

# 复制 Qt 插件目录（platforms, imageformats等必须在与DLL同级的位置）
$qtPluginDirs = @("platforms", "imageformats", "styles")
foreach ($dir in $qtPluginDirs) {
    $sourcePath = Join-Path $qtDllPath $dir
    $destPath = Join-Path $aePath $dir
    if (Test-Path $sourcePath) {
        Copy-Item -Path $sourcePath -Destination $destPath -Recurse -Force
        Write-Host "Copied $dir directory to Plug-ins"
    }
}

Write-Host "`nDeployment complete!" -ForegroundColor Green
Write-Host "Files copied to: $aePath"
Write-Host "`nIMPORTANT: All files are now in the Plug-ins directory directly."
Write-Host "Please restart After Effects to load the plugin."
