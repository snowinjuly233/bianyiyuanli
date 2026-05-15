$ErrorActionPreference = 'Stop'
if (Get-Variable -Name PSNativeCommandUseErrorActionPreference -ErrorAction SilentlyContinue) {
    $PSNativeCommandUseErrorActionPreference = $false
}

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$testsRoot = $scriptDir
$repoRoot = Split-Path -Parent (Split-Path -Parent $testsRoot)
$buildDir = Join-Path $repoRoot 'build-x64-debug'
$generatedDir = Join-Path $repoRoot 'generated-yacc-tests'
$generatedBuildDir = Join-Path $generatedDir 'build-x64'
$outputDir = Join-Path $testsRoot 'results'
$sampleInput = Join-Path $repoRoot 'specs\samples\minic_demo.c'
$invalidInput = Join-Path $repoRoot 'specs\samples\minic_invalid.c'
$sampleYacc = Join-Path $repoRoot 'specs\minic.y'
$sampleLex = Join-Path $repoRoot 'specs\minic.l'
$legacyValidOutput = Join-Path $repoRoot 'tests\seuyacc_valid_output.txt'
$legacyInvalidOutput = Join-Path $repoRoot 'tests\seuyacc_invalid_output.txt'
$vswhere = 'C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe'

if (!(Test-Path $vswhere)) {
    throw "vswhere not found at $vswhere"
}

$vsInfo = & $vswhere -latest -products * -format json | ConvertFrom-Json
if (!$vsInfo) {
    throw 'Visual Studio installation not found'
}

$installPath = $vsInfo[0].installationPath
$vsDevCmd = Join-Path $installPath 'Common7\Tools\VsDevCmd.bat'
$cmake = Join-Path $installPath 'Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe'

New-Item -ItemType Directory -Force -Path $outputDir | Out-Null
New-Item -ItemType Directory -Force -Path $generatedDir | Out-Null

& cmd.exe /c "call `"$vsDevCmd`" -arch=amd64 -host_arch=amd64 >nul 2>&1 && `"$cmake`" -S `"$repoRoot`" -B `"$buildDir`" -G Ninja -DCMAKE_BUILD_TYPE=Debug" | Out-Null
if ($LASTEXITCODE -ne 0) {
    throw 'CMake configure failed'
}

& cmd.exe /c "call `"$vsDevCmd`" -arch=amd64 -host_arch=amd64 >nul 2>&1 && `"$cmake`" --build `"$buildDir`" --config Debug --target seulex seuyacc" | Out-Null
if ($LASTEXITCODE -ne 0) {
    throw 'Failed to build seulex or seuyacc'
}

$seulex = Join-Path $buildDir 'bin\seulex.exe'
$seuyacc = Join-Path $buildDir 'bin\seuyacc.exe'
if (!(Test-Path $seulex)) {
    throw "seulex executable not found at $seulex"
}
if (!(Test-Path $seuyacc)) {
    throw "seuyacc executable not found at $seuyacc"
}

$scannerStdoutPath = Join-Path $outputDir 'scanner_generator.stdout.txt'
$scannerStderrPath = Join-Path $outputDir 'scanner_generator.stderr.txt'
$scannerProcess = Start-Process -FilePath $seulex `
    -ArgumentList @($sampleLex, $sampleInput, $generatedDir) `
    -RedirectStandardOutput $scannerStdoutPath `
    -RedirectStandardError $scannerStderrPath `
    -WorkingDirectory $repoRoot `
    -NoNewWindow `
    -Wait `
    -PassThru
if ($scannerProcess.ExitCode -ne 0) {
    throw "Failed to regenerate scanner artifacts, exit code $($scannerProcess.ExitCode)"
}

$generateStdoutPath = Join-Path $outputDir 'generator.stdout.txt'
$generateStderrPath = Join-Path $outputDir 'generator.stderr.txt'
$generateProcess = Start-Process -FilePath $seuyacc `
    -ArgumentList @($sampleYacc, $sampleInput, $sampleLex, $generatedDir) `
    -RedirectStandardOutput $generateStdoutPath `
    -RedirectStandardError $generateStderrPath `
    -WorkingDirectory $repoRoot `
    -NoNewWindow `
    -Wait `
    -PassThru
if ($generateProcess.ExitCode -ne 0) {
    throw "Failed to regenerate parser artifacts, exit code $($generateProcess.ExitCode)"
}

& cmd.exe /c "call `"$vsDevCmd`" -arch=amd64 -host_arch=amd64 >nul 2>&1 && `"$cmake`" -S `"$generatedDir`" -B `"$generatedBuildDir`" -G Ninja -DCMAKE_BUILD_TYPE=Debug" | Out-Null
if ($LASTEXITCODE -ne 0) {
    throw 'Failed to configure generated parser project'
}

& cmd.exe /c "call `"$vsDevCmd`" -arch=amd64 -host_arch=amd64 >nul 2>&1 && `"$cmake`" --build `"$generatedBuildDir`" --config Debug" | Out-Null
if ($LASTEXITCODE -ne 0) {
    throw 'Failed to build generated parser project'
}

$generatedParser = Join-Path $generatedBuildDir 'minic_parser.exe'
if (!(Test-Path $generatedParser)) {
    throw "generated parser executable not found at $generatedParser"
}

$validStdoutPath = Join-Path $outputDir 'minic_valid_output.txt'
$validStderrPath = Join-Path $outputDir 'minic_valid_error.txt'
$invalidStdoutPath = Join-Path $outputDir 'minic_invalid_output.txt'
$invalidStderrPath = Join-Path $outputDir 'minic_invalid_error.txt'

$validProcess = Start-Process -FilePath $generatedParser `
    -ArgumentList @($sampleInput) `
    -RedirectStandardOutput $validStdoutPath `
    -RedirectStandardError $validStderrPath `
    -NoNewWindow `
    -Wait `
    -PassThru

$invalidProcess = Start-Process -FilePath $generatedParser `
    -ArgumentList @($invalidInput) `
    -RedirectStandardOutput $invalidStdoutPath `
    -RedirectStandardError $invalidStderrPath `
    -NoNewWindow `
    -Wait `
    -PassThru

$validText = ((Get-Content -Raw $validStdoutPath) + (Get-Content -Raw $validStderrPath))
$invalidText = ((Get-Content -Raw $invalidStdoutPath) + (Get-Content -Raw $invalidStderrPath))

Set-Content -Path $legacyValidOutput -Value (Get-Content -Raw $validStdoutPath) -Encoding UTF8
Set-Content -Path $legacyInvalidOutput -Value (Get-Content -Raw $invalidStdoutPath) -Encoding UTF8

$checks = @(
    @{ Check = 'valid program accepted'; Result = ($validProcess.ExitCode -eq 0) -and $validText.Contains('ACCEPT') },
    @{ Check = 'valid program builds AST'; Result = $validText.Contains('AST root: Program') },
    @{ Check = 'valid parser report includes state counts'; Result = $validText.Contains('LR(1) states:') -and $validText.Contains('LALR(1) states:') },
    @{ Check = 'invalid program rejected'; Result = ($invalidProcess.ExitCode -ne 0) -and $invalidText.Contains('REJECT') },
    @{ Check = 'invalid program prints diagnostic'; Result = $invalidText.Contains('error:') }
)

$reportPath = Join-Path $outputDir 'yacc_test_results.md'
$reportLines = @(
    '# SeuYacc Test Results',
    '',
    '| Check | Result |',
    '| --- | --- |'
)

foreach ($check in $checks) {
    $reportLines += "| $($check.Check) | $(if ($check.Result) { 'PASS' } else { 'FAIL' }) |"
}

Set-Content -Path $reportPath -Value $reportLines -Encoding UTF8

$failed = $checks | Where-Object { -not $_.Result }
if ($failed) {
    throw "Yacc tests failed. See $reportPath"
}

Write-Output "Yacc tests passed. Report: $reportPath"
