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

& cmd.exe /c "call `"$vsDevCmd`" -arch=amd64 -host_arch=amd64 >nul 2>&1 && `"$cmake`" --build `"$buildDir`" --config Debug --target frontend" | Out-Null
if ($LASTEXITCODE -ne 0) {
    throw 'frontend build failed'
}

$frontend = Join-Path $buildDir 'bin\frontend.exe'
if (!(Test-Path $frontend)) {
    throw "frontend executable not found at $frontend"
}

$cases = @(
    @{
        Name = 'feature_coverage'
        InputPath = Join-Path $repoRoot 'tests\ir\inputs\feature_coverage.c'
        ExpectedExit = 0
        ExpectedFragments = @('Tokens:', 'AST root: Program', 'Semantic analysis: OK', 'IR quads:', 'Basic blocks:', 'ACCEPT')
        ExpectedOutputFiles = @('feature_coverage.ast.txt', 'feature_coverage.ast.md', 'feature_coverage.ir.txt', 'feature_coverage.blocks.txt')
    },
    @{
        Name = 'minic_invalid'
        InputPath = Join-Path $repoRoot 'specs\samples\minic_invalid.c'
        ExpectedExit = 1
        ExpectedFragments = @('REJECT', 'error:')
        ExpectedOutputFiles = @()
    },
    @{
        Name = 'error_var_redefinition'
        InputPath = Join-Path $repoRoot 'tests\semantic\inputs\error_var_redefinition.c'
        ExpectedExit = 1
        ExpectedFragments = @('AST root: Program', 'Semantic analysis: FAILED', 'REJECT', 'redefinition of variable')
        ExpectedOutputFiles = @('error_var_redefinition.ast.txt', 'error_var_redefinition.ast.md')
    }
)

$resultRows = @()
foreach ($case in $cases) {
    $stdoutPath = Join-Path $outputDir ($case.Name + '.stdout.txt')
    $stderrPath = Join-Path $outputDir ($case.Name + '.stderr.txt')
    $caseOutputDir = Join-Path $programOutputDir $case.Name

    New-Item -ItemType Directory -Force -Path $caseOutputDir | Out-Null

    $process = Start-Process -FilePath $frontend `
        -ArgumentList @($case.InputPath, $lexSpec, $yaccSpec, $caseOutputDir) `
        -RedirectStandardOutput $stdoutPath `
        -RedirectStandardError $stderrPath `
        -NoNewWindow `
        -Wait `
        -PassThru

    $combined = ''
    if (Test-Path $stdoutPath) {
        $combined += Get-Content -Raw $stdoutPath
    }
    if (Test-Path $stderrPath) {
        $combined += Get-Content -Raw $stderrPath
    }

    $resultRows += [PSCustomObject]@{
        Case = $case.Name
        Check = 'exit code'
        Passed = ($process.ExitCode -eq $case.ExpectedExit)
    }

    foreach ($fragment in $case.ExpectedFragments) {
        $resultRows += [PSCustomObject]@{
            Case = $case.Name
            Check = "output contains $fragment"
            Passed = $combined.Contains($fragment)
        }
    }

    foreach ($fileName in $case.ExpectedOutputFiles) {
        $resultRows += [PSCustomObject]@{
            Case = $case.Name
            Check = "output file $fileName"
            Passed = (Test-Path (Join-Path $caseOutputDir $fileName))
        }
    }
}

$reportPath = Join-Path $outputDir 'frontend_test_results.md'
$reportLines = @(
    '# Frontend End-to-End Test Results',
    '',
    '| Case | Check | Result |',
    '| --- | --- | --- |'
)

foreach ($row in $resultRows) {
    $reportLines += "| $($row.Case) | $($row.Check) | $(if ($row.Passed) { 'PASS' } else { 'FAIL' }) |"
}

Set-Content -Path $reportPath -Value $reportLines -Encoding UTF8

$failed = $resultRows | Where-Object { -not $_.Passed }
if ($failed) {
    throw "Frontend end-to-end tests failed. See $reportPath"
}

Write-Output "Frontend end-to-end tests passed. Report: $reportPath"
