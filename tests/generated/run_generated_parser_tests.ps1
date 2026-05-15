$ErrorActionPreference = 'Stop'
if (Get-Variable -Name PSNativeCommandUseErrorActionPreference -ErrorAction SilentlyContinue) {
    $PSNativeCommandUseErrorActionPreference = $false
}

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$testsRoot = $scriptDir
$repoRoot = Split-Path -Parent (Split-Path -Parent $testsRoot)
$buildDir = Join-Path $repoRoot 'build-x64-debug'
$generatedDir = Join-Path $repoRoot 'generated'
$generatedBuildDir = Join-Path $generatedDir 'build-x64'
$outputDir = Join-Path $testsRoot 'results'
$stdoutPath = Join-Path $outputDir 'generated_parser.stdout.txt'
$stderrPath = Join-Path $outputDir 'generated_parser.stderr.txt'
$reportPath = Join-Path $outputDir 'generated_parser_test_results.md'
$sampleInput = Join-Path $repoRoot 'specs\samples\minic_demo.c'
$sampleYacc = Join-Path $repoRoot 'specs\minic.y'
$sampleLex = Join-Path $repoRoot 'specs\minic.l'
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

& cmd.exe /c "call `"$vsDevCmd`" -arch=amd64 -host_arch=amd64 >nul 2>&1 && `"$cmake`" --build `"$buildDir`" --target seuyacc --config Debug" | Out-Null
if ($LASTEXITCODE -ne 0) {
    throw 'Failed to build seuyacc'
}

$seuyacc = Join-Path $buildDir 'bin\seuyacc.exe'
$generateProcess = Start-Process -FilePath $seuyacc `
    -ArgumentList @($sampleYacc, $sampleInput, $sampleLex, $generatedDir) `
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
$runProcess = Start-Process -FilePath $generatedParser `
    -ArgumentList @($sampleInput) `
    -RedirectStandardOutput $stdoutPath `
    -RedirectStandardError $stderrPath `
    -NoNewWindow `
    -Wait `
    -PassThru

$stdoutText = if (Test-Path $stdoutPath) { Get-Content -Raw $stdoutPath } else { '' }
$stderrText = if (Test-Path $stderrPath) { Get-Content -Raw $stderrPath } else { '' }
$combined = $stdoutText + $stderrText

$checks = @(
    @{ Name = 'generated parser exits successfully'; Passed = ($runProcess.ExitCode -eq 0); Expected = 'exit 0' },
    @{ Name = 'generated parser accepts sample'; Passed = $combined.Contains('ACCEPT'); Expected = 'ACCEPT' },
    @{ Name = 'generated parser builds AST'; Passed = $combined.Contains('AST root: Program'); Expected = 'AST root: Program' }
)

$reportLines = @(
    '# Generated Parser Test Results',
    '',
    '| Check | Expected | Result |',
    '| --- | --- | --- |'
)

foreach ($check in $checks) {
    $reportLines += "| $($check.Name) | ``$($check.Expected)`` | $(if ($check.Passed) { 'PASS' } else { 'FAIL' }) |"
}

Set-Content -Path $reportPath -Value $reportLines

$failed = $checks | Where-Object { -not $_.Passed }
if ($failed) {
    throw "Generated parser tests failed. See $reportPath"
}

Write-Output "Generated parser tests passed. Report: $reportPath"
