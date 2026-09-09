param([string]$MySqlRoot = 'C:\Program Files\MySQL\MySQL Server 8.0')
$ErrorActionPreference = 'Stop'
Set-Location (Split-Path $PSScriptRoot -Parent)
if (!(Get-Command cmake -ErrorAction SilentlyContinue)) { throw 'Install CMake and put it on PATH.' }
if (Get-Command g++ -ErrorAction SilentlyContinue) {
  cmake -S . -B build -G 'MinGW Makefiles' -DCMAKE_BUILD_TYPE=Release "-DMYSQL_ROOT=$MySqlRoot"
} else {
  cmake -S . -B build "-DMYSQL_ROOT=$MySqlRoot"
}
if ($LASTEXITCODE -ne 0) { throw 'CMake configuration failed.' }
cmake --build build --config Release --parallel 8
if ($LASTEXITCODE -ne 0) { throw 'Build failed.' }
ctest --test-dir build -C Release --output-on-failure
if ($LASTEXITCODE -ne 0) { throw 'Tests failed.' }
