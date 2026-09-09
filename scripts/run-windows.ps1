$ErrorActionPreference = 'Stop'
Set-Location (Split-Path $PSScriptRoot -Parent)
# The tested MSYS2 compiler runtime must be on PATH.
$compiler = Get-Command g++ -ErrorAction SilentlyContinue
if ($compiler) { $env:PATH = (Split-Path $compiler.Source) + ';' + $env:PATH }
$program = if (Test-Path 'build/saturday.exe') { 'build/saturday.exe' } else { 'build/Release/saturday.exe' }
if (!(Test-Path $program)) { throw 'Run scripts/build-windows.ps1 first.' }
& $program @args
exit $LASTEXITCODE
