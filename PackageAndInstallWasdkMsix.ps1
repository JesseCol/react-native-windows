

$ErrorActionPreference='Stop'

Write-Host "Packaging WindowsAppSDK" -ForegroundColor Cyan

# microsoft-ui-xaml-lift\BuildOutput\WindowsAppSDK\fwp\MSIX\win10-x64\Microsoft.WindowsAppRuntime.1.7-Dev.msix
# Build a version string 11.<date>.<time>
$version = "11." + (Get-Date -Format "yyMMdd.HHmm")
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

$pkg = "Microsoft.WindowsAppRuntime.1.7-Dev"

Write-Host "Remove all WindowsAppRutime pacakges..." -ForegroundColor Cyan
$packagesToRemove = @()

foreach ($p in (Get-AppxPackage *))
{
    foreach ($d in $p.Dependencies)
    {
        if ($d.Name -eq $pkg)
        {
            $packagesToRemove += $p.PackageFullName
            break;
        }
    }
}

foreach ($p in (Get-AppxPackage $pkg))
{
    $packagesToRemove += $p.PackageFullName
}


"Packages to remove:"
$packagesToRemove

foreach ($p in $packagesToRemove)
{
    Write-Host "Removing package $p"
    Remove-AppxPackage -Package $p
}

if ($packagesToRemove.Count -eq 0)
{
    Write-Host "No packages to remove."
}

Add-AppxPackage "$PSScriptRoot\microsoft-ui-xaml-lift\BuildOutput\WindowsAppSDK\fwp\MSIX\win10-x64\Microsoft.WindowsAppRuntime.1.7-Dev.msix"

