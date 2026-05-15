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
        Name = 'minic_demo'
        InputPath = Join-Path $repoRoot 'specs\samples\minic_demo.c'
        ExpectedTextFragments = @(
            'Program',
            'FunctionDecl name="main" return=int',
            'AssignExpr target="x"',
            'IfStmt',
            'ReturnStmt'
        )
        ExpectedMarkdownFragments = @(
            '# AST',
            '- `Program`',
            '`FunctionDecl name="main" return=int`',
            '`IfStmt`'
        )
    },
    @{
        Name = 'feature_coverage'
        InputPath = Join-Path $repoRoot 'tests\ir\inputs\feature_coverage.c'
        ExpectedTextFragments = @(
            'VarDecl name="seed" type=int',
            'FunctionDecl name="twice" return=int',
            'CallExpr callee="twice"',
            'Else',
            'WhileStmt'
        )
        ExpectedMarkdownFragments = @(
            '# AST',
            '`VarDecl name="seed" type=int`',
            '`CallExpr callee="twice"`',
            '`WhileStmt`'
        )
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

    $inputStem = [System.IO.Path]::GetFileNameWithoutExtension($case.InputPath)
    $astTextPath = Join-Path $caseOutputDir ($inputStem + '.ast.txt')
    $astMarkdownPath = Join-Path $caseOutputDir ($inputStem + '.ast.md')

    if (!(Test-Path $astTextPath)) {
        throw "AST text output not found at $astTextPath"
    }
    if (!(Test-Path $astMarkdownPath)) {
        throw "AST markdown output not found at $astMarkdownPath"
    }

    $combined = ''
    if (Test-Path $stdoutPath) {
        $combined += Get-Content -Raw $stdoutPath
    }
    if (Test-Path $stderrPath) {
        $combined += Get-Content -Raw $stderrPath
    }

    $astText = Get-Content -Raw $astTextPath
    $astMarkdown = Get-Content -Raw $astMarkdownPath

    $checks = @(
        @{ Label = 'frontend accept'; Passed = ($process.ExitCode -eq 0) -and $combined.Contains('ACCEPT') },
        @{ Label = 'ast root'; Passed = $combined.Contains('AST root: Program') }
    )

    foreach ($fragment in $case.ExpectedTextFragments) {
        $checks += @{ Label = "text fragment $fragment"; Passed = $astText.Contains($fragment) }
    }
    foreach ($fragment in $case.ExpectedMarkdownFragments) {
        $checks += @{ Label = "markdown fragment $fragment"; Passed = $astMarkdown.Contains($fragment) }
    }

    foreach ($check in $checks) {
        $resultRows += [PSCustomObject]@{
            Case = $case.Name
            Check = $check.Label
            Passed = $check.Passed
        }
    }
}

$reportPath = Join-Path $outputDir 'ast_test_results.md'
$reportLines = @(
    '# AST Test Results',
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
    throw "AST tests failed. See $reportPath"
}

Write-Output "AST tests passed. Report: $reportPath"
