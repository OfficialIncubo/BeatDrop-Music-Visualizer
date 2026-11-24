$ErrorActionPreference = 'Stop'

$dxSdkPath = 'C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)'
$installerPath = Join-Path $env:RUNNER_TEMP 'DXSDK_Jun10.exe'
$downloadUrl = 'https://download.microsoft.com/download/a/e/7/ae743f1f-632b-4809-87a9-aa1bb3458e31/DXSDK_Jun10.exe'
$expectedSha256 = '705271DC83BFEE54D9B94E028426E288D5F070784B7446D164F48ECFBB2A02CB'

if (-not (Test-Path -LiteralPath $dxSdkPath)) {
    Write-Host 'Downloading the DirectX SDK installer from Microsoft...'
    Invoke-WebRequest -Uri $downloadUrl -OutFile $installerPath

    $actualSha256 = (Get-FileHash -LiteralPath $installerPath -Algorithm SHA256).Hash
    if ($actualSha256 -ne $expectedSha256) {
        throw "DirectX SDK installer SHA-256 mismatch. Expected $expectedSha256, got $actualSha256."
    }

    Write-Host 'Installing the DirectX SDK...'
    $installer = Start-Process -FilePath $installerPath -ArgumentList '/U' -Wait -PassThru
    if ($installer.ExitCode -notin @(0, 1023)) {
        throw "DirectX SDK installer failed with exit code $($installer.ExitCode)."
    }
}

# Do not let a successful download or installer exit code mask an incomplete
# install: the build needs headers and libraries for both target architectures.
$requiredFiles = @(
    (Join-Path $dxSdkPath 'Include\d3dx9.h'),
    (Join-Path $dxSdkPath 'Lib\x86\d3dx9.lib'),
    (Join-Path $dxSdkPath 'Lib\x64\d3dx9.lib')
)
$missingFiles = @($requiredFiles | Where-Object { -not (Test-Path -LiteralPath $_) })
if ($missingFiles.Count -gt 0) {
    throw "The DirectX SDK installation is incomplete. Missing: $($missingFiles -join ', ')"
}

"DXSDK_DIR=$dxSdkPath" | Out-File -FilePath $env:GITHUB_ENV -Encoding utf8 -Append