$ErrorActionPreference = 'Stop'

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$repoRoot = Split-Path -Parent $scriptDir
$outputDir = Join-Path $scriptDir 'results'

New-Item -ItemType Directory -Force -Path $outputDir | Out-Null

$scripts = @(
    @{ Name = 'lex'; Path = Join-Path $scriptDir 'lex\run_lex_tests.ps1'; Report = Join-Path $scriptDir 'lex\results\lex_test_results.md' },
    @{ Name = 'yacc'; Path = Join-Path $scriptDir 'yacc\run_yacc_tests.ps1'; Report = Join-Path $scriptDir 'yacc\results\yacc_test_results.md' },
    @{ Name = 'ast'; Path = Join-Path $scriptDir 'ast\run_ast_tests.ps1'; Report = Join-Path $scriptDir 'ast\results\ast_test_results.md' },
    @{ Name = 'semantic'; Path = Join-Path $scriptDir 'semantic\run_semantic_tests.ps1'; Report = Join-Path $scriptDir 'semantic\results\semantic_test_results.md' },
    @{ Name = 'ir'; Path = Join-Path $scriptDir 'ir\run_ir_tests.ps1'; Report = Join-Path $scriptDir 'ir\results\ir_test_results.md' },
    @{ Name = 'frontend'; Path = Join-Path $scriptDir 'frontend\run_frontend_tests.ps1'; Report = Join-Path $scriptDir 'frontend\results\frontend_test_results.md' }
)

$rows = @()
foreach ($script in $scripts) {
    & powershell -ExecutionPolicy Bypass -File $script.Path
    if ($LASTEXITCODE -ne 0) {
        throw "Stage 6 test step failed: $($script.Name)"
    }

    $rows += [PSCustomObject]@{
        Step = $script.Name
        Report = $script.Report
    }
}

$reportPath = Join-Path $outputDir 'stage6_test_results.md'
$reportLines = @(
    '# Stage 6 Test Summary',
    '',
    '| Step | Report |',
    '| --- | --- |'
)

foreach ($row in $rows) {
    $relativeReport = $row.Report.Replace($repoRoot.TrimEnd('\') + '\', '').Replace('\', '/')
    $reportLines += "| $($row.Step) | ``$relativeReport`` |"
}

Set-Content -Path $reportPath -Value $reportLines -Encoding UTF8
Write-Output "Stage 6 test suite passed. Report: $reportPath"
