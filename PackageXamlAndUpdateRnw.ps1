$ErrorActionPreference = "Stop"

# Build a version string 11.<date>.<time>
$version = "11." + (Get-Date -Format "yyMMdd.Hmm")
$winAppSdkPackageVersion = "999.0.0-mock-$($version)-x64-Debug"
Write-Host "Version: $version"
Write-Host "WinAppSDK package Version: $winAppSdkPackageVersion"

pushd microsoft-ui-xaml-lift

$cmd = ".\pack.cmd /version $version"
Write-Host "Running: $cmd"

cmd.exe /c $cmd 

if (!$?) {
    Write-Error "Failed to pack Microsoft.UI.Xaml"
    exit 1
}

popd

Write-Host "Pushing WinAppSDK package to WinUI-Test feed"
nuget push -Source WinUI-Test "microsoft-ui-xaml-lift\PackageStore\Microsoft.WindowsAppSDK.$winAppSdkPackageVersion.nupkg" -ApiKey AzDepOps
if (!$?) {
    Write-Error "Failed to push WinAppSDK package."
    exit 1
}

Write-Host "Updating playground-composition.sln with WinAppSDK package version $winAppSdkPackageVersion"

$propsFile = "$PSScriptRoot\vnext\PropertySheets\WinUI.props"
# Update file so that the <WinUI3ExpVersion> tag contains $winAppSdkPackageVersion
(Get-Content $propsFile) | ForEach-Object { $_ -replace "<WinUI3ExpVersion>.*</WinUI3ExpVersion>", "<WinUI3ExpVersion>$winAppSdkPackageVersion</WinUI3ExpVersion>" } | Set-Content $propsFile

Write-Host "Attempting to restore playground-composition.sln"

& "$PSScriptRoot\vnext\Scripts\NuGetRestoreForceEvaluateAllSolutions.ps1"
if (!$?) {
    Write-Error "Failed to restore solutions."
    exit 1
}

Write-Host "New version: $version" -ForegroundColor Green
Write-Host "New WinAppSDK package Version: $winAppSdkPackageVersion" -ForegroundColor Green
