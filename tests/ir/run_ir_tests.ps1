$ErrorActionPreference = 'Stop'
if (Get-Variable -Name PSNativeCommandUseErrorActionPreference -ErrorAction SilentlyContinue) {
    $PSNativeCommandUseErrorActionPreference = $false
}

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$testsRoot = $scriptDir
$repoRoot = Split-Path -Parent (Split-Path -Parent $testsRoot)
$buildDir = Join-Path $repoRoot 'build-x64-debug'
$outputDir = Join-Path $testsRoot 'results'
$programOutputDir = Join-Path $outputDir 'program_outputs'
$inputDir = Join-Path $testsRoot 'inputs'
$expectedDir = Join-Path $testsRoot 'expected'
$lexSpec = Join-Path $repoRoot 'specs\minic.l'
$yaccSpec = Join-Path $repoRoot 'specs\minic.y'
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
New-Item -ItemType Directory -Force -Path $programOutputDir | Out-Null

& cmd.exe /c "call `"$vsDevCmd`" -arch=amd64 -host_arch=amd64 >nul 2>&1 && `"$cmake`" -S `"$repoRoot`" -B `"$buildDir`" -G Ninja -DCMAKE_BUILD_TYPE=Debug" | Out-Null
if ($LASTEXITCODE -ne 0) {
    throw 'CMake configure failed'
}

& cmd.exe /c "call `"$vsDevCmd`" -arch=amd64 -host_arch=amd64 >nul 2>&1 && `"$cmake`" --build `"$buildDir`" --config Debug" | Out-Null
if ($LASTEXITCODE -ne 0) {
    throw 'Project build failed'
}

$frontend = Join-Path $buildDir 'bin\frontend.exe'
if (!(Test-Path $frontend)) {
    throw "frontend executable not found at $frontend"
}

$caseName = 'feature_coverage'
$inputPath = Join-Path $inputDir ($caseName + '.c')
$stdoutPath = Join-Path $outputDir ($caseName + '.stdout.txt')
$stderrPath = Join-Path $outputDir ($caseName + '.stderr.txt')
$caseOutputDir = Join-Path $programOutputDir $caseName

New-Item -ItemType Directory -Force -Path $caseOutputDir | Out-Null

$process = Start-Process -FilePath $frontend `
    -ArgumentList @($inputPath, $lexSpec, $yaccSpec, $caseOutputDir) `
    -RedirectStandardOutput $stdoutPath `
    -RedirectStandardError $stderrPath `
    -NoNewWindow `
    -Wait `
    -PassThru
$exitCode = $process.ExitCode
if ($exitCode -ne 0) {
    throw "IR feature coverage case failed with exit code $exitCode"
}

$irPath = Join-Path $caseOutputDir ($caseName + '.ir.txt')
$blockPath = Join-Path $caseOutputDir ($caseName + '.blocks.txt')
$expectedIrPath = Join-Path $expectedDir ($caseName + '.ir.txt')
$expectedBlockPath = Join-Path $expectedDir ($caseName + '.blocks.txt')

if (!(Test-Path $irPath)) {
    throw "IR output file not found at $irPath"
}
if (!(Test-Path $blockPath)) {
    throw "Basic block output file not found at $blockPath"
}

$irText = Get-Content -Raw $irPath
$blockText = Get-Content -Raw $blockPath
$expectedIrText = (Get-Content -Raw $expectedIrPath).Replace("`r`n", "`n").TrimEnd("`n")
$expectedBlockText = (Get-Content -Raw $expectedBlockPath).Replace("`r`n", "`n").TrimEnd("`n")
$normalizedIrText = $irText.Replace("`r`n", "`n").TrimEnd("`n")
$normalizedBlockText = $blockText.Replace("`r`n", "`n").TrimEnd("`n")

$checks = @(
    @{ Name = 'frontend exits successfully'; Expected = 'exit 0'; Passed = ($exitCode -eq 0) },
    @{ Name = 'IR golden output matches'; Expected = $expectedIrPath; Passed = ($normalizedIrText -eq $expectedIrText) },
    @{ Name = 'basic block golden output matches'; Expected = $expectedBlockPath; Passed = ($normalizedBlockText -eq $expectedBlockText) }
)

$resultRows = @()
foreach ($check in $checks) {
    $resultRows += [PSCustomObject]@{
        Check = $check.Name
        Expected = $check.Expected
        Passed = $check.Passed
    }
}

$reportPath = Join-Path $outputDir 'ir_test_results.md'
$reportLines = @(
    '# IR Test Results',
    '',
    '| Check | Expected | Result |',
    '| --- | --- | --- |'
)

foreach ($row in $resultRows) {
    $reportLines += "| $($row.Check) | ``$($row.Expected)`` | $(if ($row.Passed) { 'PASS' } else { 'FAIL' }) |"
}

Set-Content -Path $reportPath -Value $reportLines

$failed = $resultRows | Where-Object { -not $_.Passed }
if ($failed) {
    throw "IR tests failed. See $reportPath"
}

Write-Output "IR tests passed. Report: $reportPath"
