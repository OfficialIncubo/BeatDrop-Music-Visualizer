$ErrorActionPreference = 'Stop'

$dxSdkPath = 'C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)'
$chocoOutput = & choco install directx-sdk -y --no-progress 2>&1
$chocoExitCode = $LASTEXITCODE
$chocoOutput | ForEach-Object { Write-Host $_ }
$chocoOutputText = $chocoOutput -join "`n"

# Chocolatey can print resolver errors while returning success. Treat resolver
# failures and transient feed errors as install failures instead of allowing
# the build to continue without the legacy SDK.
if ($chocoOutputText -match '(?i)NuGetResolverInputException' -or
    $chocoOutputText -match '(?i)Unable to find package.*directx-sdk' -or
    $chocoOutputText -match '(?i)Response status code does not indicate success:\s*503') {
    throw "Chocolatey could not resolve or download the DirectX SDK package. Check the Chocolatey Community feed and retry."
}

if ($chocoExitCode -ne 0) {
    throw "Chocolatey failed to install directx-sdk (exit code $chocoExitCode)."
}

$requiredFiles = @(
    (Join-Path $dxSdkPath 'Include\d3dx9.h'),
    (Join-Path $dxSdkPath 'Lib\x86\d3dx9.lib'),
    (Join-Path $dxSdkPath 'Lib\x64\d3dx9.lib')
)
$missingFiles = @($requiredFiles | Where-Object { -not (Test-Path -LiteralPath $_) })
if ($missingFiles.Count -gt 0) {
    throw "Chocolatey reported success, but the DirectX SDK is incomplete. Missing: $($missingFiles -join ', ')"
}

"DXSDK_DIR=$dxSdkPath" | Out-File -FilePath $env:GITHUB_ENV -Encoding utf8 -Append