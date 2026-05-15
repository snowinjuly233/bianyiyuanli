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

if (!(Test-Path $vsDevCmd)) {
    throw "VsDevCmd.bat not found at $vsDevCmd"
}
if (!(Test-Path $cmake)) {
    throw "cmake.exe not found at $cmake"
}

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

$cases = @(
    @{ Name = 'valid_minimal'; ExpectedExit = 0; ExpectedText = 'ACCEPT' },
    @{ Name = 'valid_scope_shadowing'; ExpectedExit = 0; ExpectedText = 'ACCEPT' },
    @{ Name = 'error_var_redefinition'; ExpectedExit = 1; ExpectedText = 'redefinition of variable' },
    @{ Name = 'error_param_redefinition'; ExpectedExit = 1; ExpectedText = 'redefinition of parameter' },
    @{ Name = 'error_function_redefinition'; ExpectedExit = 1; ExpectedText = 'redefinition of function' },
    @{ Name = 'error_undeclared_identifier'; ExpectedExit = 1; ExpectedText = 'assignment to undeclared identifier' },
    @{ Name = 'error_call_undeclared_function'; ExpectedExit = 1; ExpectedText = 'call to undeclared function' },
    @{ Name = 'error_call_non_function'; ExpectedExit = 1; ExpectedText = 'call of non-function' },
    @{ Name = 'error_arg_count'; ExpectedExit = 1; ExpectedText = 'function call argument count mismatch' },
    @{ Name = 'error_arg_type'; ExpectedExit = 1; ExpectedText = 'function argument type mismatch' },
    @{ Name = 'error_assign_type'; ExpectedExit = 1; ExpectedText = 'assignment type mismatch' },
    @{ Name = 'error_init_type'; ExpectedExit = 1; ExpectedText = 'initialization type mismatch for variable' },
    @{ Name = 'error_condition_type'; ExpectedExit = 1; ExpectedText = 'if condition must be scalar type' },
    @{ Name = 'error_while_condition_type'; ExpectedExit = 1; ExpectedText = 'while condition must be scalar type' },
    @{ Name = 'error_return_void_value'; ExpectedExit = 1; ExpectedText = 'void function must not return a value' },
    @{ Name = 'error_return_missing_value'; ExpectedExit = 1; ExpectedText = 'non-void function must return a value' },
    @{ Name = 'error_return_type_mismatch'; ExpectedExit = 1; ExpectedText = 'return type mismatch' }
)

$resultRows = @()

foreach ($case in $cases) {
    $inputPath = Join-Path $inputDir ($case.Name + '.c')
    $stdoutPath = Join-Path $outputDir ($case.Name + '.stdout.txt')
    $stderrPath = Join-Path $outputDir ($case.Name + '.stderr.txt')
    $caseOutputDir = Join-Path $programOutputDir $case.Name

    New-Item -ItemType Directory -Force -Path $caseOutputDir | Out-Null

    $stdout = [System.Collections.Generic.List[string]]::new()
    $stderr = [System.Collections.Generic.List[string]]::new()

    $process = Start-Process -FilePath $frontend `
        -ArgumentList @($inputPath, $lexSpec, $yaccSpec, $caseOutputDir) `
        -RedirectStandardOutput $stdoutPath `
        -RedirectStandardError $stderrPath `
        -NoNewWindow `
        -Wait `
        -PassThru
    $exitCode = $process.ExitCode

    $combined = ''
    if (Test-Path $stdoutPath) {
        $combined += (Get-Content -Raw $stdoutPath)
    }
    if (Test-Path $stderrPath) {
        $combined += (Get-Content -Raw $stderrPath)
    }

    $passed = ($exitCode -eq $case.ExpectedExit) -and $combined.Contains($case.ExpectedText)
    $resultRows += [PSCustomObject]@{
        Name = $case.Name
        ExitCode = $exitCode
        ExpectedExit = $case.ExpectedExit
        ExpectedText = $case.ExpectedText
        Passed = $passed
    }
}

$reportPath = Join-Path $outputDir 'semantic_test_results.md'
$reportLines = @(
    '# Semantic Test Results',
    '',
    '| Case | Exit | Expected Exit | Expected Text | Result |',
    '| --- | ---: | ---: | --- | --- |'
)

foreach ($row in $resultRows) {
    $resultText = if ($row.Passed) { 'PASS' } else { 'FAIL' }
    $reportLines += "| $($row.Name) | $($row.ExitCode) | $($row.ExpectedExit) | ``$($row.ExpectedText)`` | $resultText |"
}

Set-Content -Path $reportPath -Value $reportLines

$failed = $resultRows | Where-Object { -not $_.Passed }
if ($failed) {
    Write-Error "Semantic tests failed. See $reportPath"
    exit 1
}

Write-Output "Semantic tests passed. Report: $reportPath"
