Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

function Get-ProjectRoot {
    return (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
}

function Resolve-CMake {
    $command = Get-Command cmake -ErrorAction SilentlyContinue
    if ($null -ne $command) {
        return $command.Source
    }

    $fallback = 'C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe'
    if (Test-Path $fallback) {
        return $fallback
    }

    throw 'cmake was not found in PATH and the Visual Studio fallback path does not exist.'
}

function Get-NormalizedText([string]$Path) {
    if (-not (Test-Path $Path)) {
        return ''
    }

    $raw = [System.IO.File]::ReadAllText((Resolve-Path $Path))
    return $raw.Replace("`r`n", "`n").TrimEnd("`n")
}

function Invoke-NativeStep([string]$FilePath, [string[]]$Arguments, [string]$Description) {
    Write-Host "[step] $Description"
    & $FilePath @Arguments
    if ($LASTEXITCODE -ne 0) {
        throw "$Description failed with exit code $LASTEXITCODE."
    }
}

function Invoke-CapturedProcess(
    [string]$FilePath,
    [string[]]$Arguments,
    [string]$OutputPath,
    [bool]$AllowFailure = $false
) {
    $stdoutPath = [System.IO.Path]::GetTempFileName()
    $stderrPath = [System.IO.Path]::GetTempFileName()

    try {
        $process = Start-Process `
            -FilePath $FilePath `
            -ArgumentList $Arguments `
            -NoNewWindow `
            -Wait `
            -PassThru `
            -RedirectStandardOutput $stdoutPath `
            -RedirectStandardError $stderrPath

        $lines = @()
        if (Test-Path $stdoutPath) {
            $lines += Get-Content $stdoutPath
        }
        if (Test-Path $stderrPath) {
            $lines += Get-Content $stderrPath
        }
        Set-Content -Path $OutputPath -Value $lines

        if (-not $AllowFailure -and $process.ExitCode -ne 0) {
            throw "Process failed unexpectedly: $FilePath $Arguments"
        }
        if ($AllowFailure -and $process.ExitCode -eq 0) {
            throw "Process succeeded unexpectedly: $FilePath $Arguments"
        }

        return [pscustomobject]@{
            ExitCode = $process.ExitCode
            Text = Get-NormalizedText $OutputPath
        }
    } finally {
        Remove-Item $stdoutPath, $stderrPath -ErrorAction SilentlyContinue
    }
}

function Convert-ToRelativePath([string]$ProjectRoot, [string]$Path) {
    $rootFull = (Resolve-Path $ProjectRoot).Path.TrimEnd('\') + '\'
    $targetFull = if ([System.IO.Path]::IsPathRooted($Path)) {
        (Resolve-Path $Path).Path
    } else {
        (Resolve-Path (Join-Path $ProjectRoot $Path)).Path
    }

    $rootUri = New-Object System.Uri($rootFull)
    $targetUri = New-Object System.Uri($targetFull)
    $relative = $rootUri.MakeRelativeUri($targetUri).ToString()
    return [System.Uri]::UnescapeDataString($relative).Replace('\', '/')
}

$projectRoot = Get-ProjectRoot
$cmake = Resolve-CMake
$buildDir = Join-Path $projectRoot 'build-lex'
$seulexExe = Join-Path $buildDir 'bin\Debug\seulex.exe'
$resultsDir = Join-Path $projectRoot 'tests\lex\results'

Push-Location $projectRoot
try {
    Invoke-NativeStep $cmake @('-S', $projectRoot, '-B', $buildDir) 'Configure main project'
    Invoke-NativeStep $cmake @('--build', $buildDir, '--config', 'Debug', '--target', 'seulex') 'Build seulex target'

    $suites = @(
        [pscustomobject]@{
            Name = 'minic'
            Spec = 'specs/minic.l'
            SampleInput = 'tests/lex/minic/inputs/keywords_and_identifiers.c'
            OutputDir = 'tests/lex/results/generated/minic'
            ScannerExe = 'minic_scanner.exe'
            GeneratorStdout = 'tests/lex/results/minic.seulex.stdout.txt'
        },
        [pscustomobject]@{
            Name = 'feature_subset'
            Spec = 'specs/lex_features.l'
            SampleInput = 'tests/lex/feature_subset/inputs/feature_mix.txt'
            OutputDir = 'tests/lex/results/generated/feature_subset'
            ScannerExe = 'lex_features_scanner.exe'
            GeneratorStdout = 'tests/lex/results/feature_subset.seulex.stdout.txt'
        }
    )

    $suiteResults = @{}
    foreach ($suite in $suites) {
        $suiteOutputDir = Join-Path $projectRoot $suite.OutputDir
        New-Item -ItemType Directory -Force -Path $suiteOutputDir | Out-Null

        $generator = Invoke-CapturedProcess `
            -FilePath $seulexExe `
            -Arguments @($suite.Spec, $suite.SampleInput, $suite.OutputDir) `
            -OutputPath (Join-Path $projectRoot $suite.GeneratorStdout)

        if ($generator.Text -notmatch 'DFA states:\s+(\d+)\s+->\s+(\d+)') {
            throw "Failed to parse DFA state counts from $($suite.GeneratorStdout)."
        }

        $originStates = [int]$Matches[1]
        $minimizedStates = [int]$Matches[2]

        $suiteBuildDir = Join-Path $suiteOutputDir 'build-local'
        Invoke-NativeStep $cmake @('-S', $suiteOutputDir, '-B', $suiteBuildDir) "Configure generated scanner for $($suite.Name)"
        Invoke-NativeStep $cmake @('--build', $suiteBuildDir, '--config', 'Debug') "Build generated scanner for $($suite.Name)"

        $scannerExePath = Join-Path $suiteBuildDir ('Debug\' + $suite.ScannerExe)
        if (-not (Test-Path $scannerExePath)) {
            throw "Generated scanner executable not found: $scannerExePath"
        }

        $suiteResults[$suite.Name] = [pscustomobject]@{
            Name = $suite.Name
            Spec = $suite.Spec
            OutputDir = $suite.OutputDir
            ScannerExePath = $scannerExePath
            OriginStates = $originStates
            MinimizedStates = $minimizedStates
            Artifacts = @(
                (Join-Path $suiteOutputDir ($suite.ScannerExe -replace '\.exe$', '.h')),
                (Join-Path $suiteOutputDir ($suite.ScannerExe -replace '\.exe$', '.cpp')),
                (Join-Path $suiteOutputDir ($suite.ScannerExe -replace '\.exe$', '_main.cpp')),
                (Join-Path $suiteOutputDir 'CMakeLists.txt'),
                (Join-Path $suiteOutputDir 'dfa.dot')
            )
        }
    }

    $cases = @(
        [pscustomobject]@{
            Suite = 'minic'
            Name = 'keywords_and_identifiers'
            Focus = 'keyword priority and identifier fallback'
            Input = 'tests/lex/minic/inputs/keywords_and_identifiers.c'
            Expected = 'tests/lex/minic/expected/keywords_and_identifiers.tokens.txt'
            Actual = 'tests/lex/results/keywords_and_identifiers.actual.txt'
            AllowFailure = $false
            ExpectedExitCode = 0
        },
        [pscustomobject]@{
            Suite = 'minic'
            Name = 'operators_and_delimiters'
            Focus = 'longest match for operators and delimiter coverage'
            Input = 'tests/lex/minic/inputs/operators_and_delimiters.c'
            Expected = 'tests/lex/minic/expected/operators_and_delimiters.tokens.txt'
            Actual = 'tests/lex/results/operators_and_delimiters.actual.txt'
            AllowFailure = $false
            ExpectedExitCode = 0
        },
        [pscustomobject]@{
            Suite = 'minic'
            Name = 'literals_and_layout'
            Focus = 'integer or float literals plus line and column tracking'
            Input = 'tests/lex/minic/inputs/literals_and_layout.c'
            Expected = 'tests/lex/minic/expected/literals_and_layout.tokens.txt'
            Actual = 'tests/lex/results/literals_and_layout.actual.txt'
            AllowFailure = $false
            ExpectedExitCode = 0
        },
        [pscustomobject]@{
            Suite = 'minic'
            Name = 'bom_input'
            Focus = 'UTF-8 BOM compatibility'
            Input = 'tests/lex/minic/inputs/bom_input.c'
            Expected = 'tests/lex/minic/expected/bom_input.tokens.txt'
            Actual = 'tests/lex/results/bom_input.actual.txt'
            AllowFailure = $false
            ExpectedExitCode = 0
        },
        [pscustomobject]@{
            Suite = 'minic'
            Name = 'invalid_char'
            Focus = 'negative case for unsupported characters'
            Input = 'tests/lex/minic/inputs/invalid_char.c'
            Expected = 'tests/lex/minic/expected/invalid_char.output.txt'
            Actual = 'tests/lex/results/invalid_char.actual.txt'
            AllowFailure = $true
            ExpectedExitCode = 1
        },
        [pscustomobject]@{
            Suite = 'feature_subset'
            Name = 'feature_mix'
            Focus = 'comment skip, wildcard, negated class, and repetition bounds'
            Input = 'tests/lex/feature_subset/inputs/feature_mix.txt'
            Expected = 'tests/lex/feature_subset/expected/feature_mix.tokens.txt'
            Actual = 'tests/lex/results/feature_mix.actual.txt'
            AllowFailure = $false
            ExpectedExitCode = 0
        }
    )

    $caseResults = @()
    $allPassed = $true
    foreach ($case in $cases) {
        $suite = $suiteResults[$case.Suite]
        $actual = Invoke-CapturedProcess `
            -FilePath $suite.ScannerExePath `
            -Arguments @($case.Input) `
            -OutputPath (Join-Path $projectRoot $case.Actual) `
            -AllowFailure $case.AllowFailure

        $expectedText = Get-NormalizedText (Join-Path $projectRoot $case.Expected)
        $actualText = Get-NormalizedText (Join-Path $projectRoot $case.Actual)
        $passed = ($expectedText -eq $actualText) -and ($actual.ExitCode -eq $case.ExpectedExitCode)
        if (-not $passed) {
            $allPassed = $false
        }

        $caseResults += [pscustomobject]@{
            Suite = $case.Suite
            Name = $case.Name
            Focus = $case.Focus
            ExpectedExitCode = $case.ExpectedExitCode
            ActualExitCode = $actual.ExitCode
            Passed = $passed
            Expected = $case.Expected
            Actual = $case.Actual
        }
    }

    $timestamp = Get-Date -Format 'yyyy-MM-dd HH:mm:ss'
    $summaryPath = Join-Path $resultsDir 'lex_test_results.md'
    $passCount = ($caseResults | Where-Object { $_.Passed }).Count
    $failCount = $caseResults.Count - $passCount

    $lines = @(
        '# SeuLex Test Results',
        '',
        "- Generated at: $timestamp",
        "- Total cases: $($caseResults.Count)",
        "- Passed: $passCount",
        "- Failed: $failCount",
        '',
        '## Generator And Minimization Summary',
        '',
        '| Spec | Original DFA States | Minimized DFA States | Standalone Scanner Build | Output Directory |',
        '| --- | ---: | ---: | --- | --- |'
    )

    foreach ($suite in $suiteResults.Values) {
        $lines += "| ``$($suite.Spec)`` | $($suite.OriginStates) | $($suite.MinimizedStates) | success | ``$(Convert-ToRelativePath $projectRoot $suite.OutputDir)`` |"
    }

    $lines += ''
    $lines += '## Case Results'
    $lines += ''
    $lines += '| Suite | Case | Focus | Expected Exit Code | Actual Exit Code | Result |'
    $lines += '| --- | --- | --- | ---: | ---: | --- |'
    foreach ($case in $caseResults) {
        $status = if ($case.Passed) { 'PASS' } else { 'FAIL' }
        $lines += "| $($case.Suite) | $($case.Name) | $($case.Focus) | $($case.ExpectedExitCode) | $($case.ActualExitCode) | $status |"
    }

    $lines += ''
    $lines += '## Raw Output Files'
    $lines += ''
    foreach ($case in $caseResults) {
        $lines += "- ``$(Convert-ToRelativePath $projectRoot $case.Actual)``"
    }

    $lines += ''
    $lines += '## Generated Artifacts'
    $lines += ''
    foreach ($suite in $suiteResults.Values) {
        $lines += "- $($suite.Name)"
        foreach ($artifact in $suite.Artifacts) {
            $lines += "  - ``$(Convert-ToRelativePath $projectRoot $artifact)``"
        }
    }

    Set-Content -Path $summaryPath -Value $lines -Encoding UTF8

    if (-not $allPassed) {
        throw "Lex regression tests failed. See $(Convert-ToRelativePath $projectRoot $summaryPath)."
    }

    Write-Host "[done] Lex regression tests passed."
    Write-Host "[done] Summary written to $(Convert-ToRelativePath $projectRoot $summaryPath)"
} finally {
    Pop-Location
}
