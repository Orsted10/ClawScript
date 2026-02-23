param(
  [string]$InstallDir = "$Env:ProgramFiles\ClawScript"
)
Write-Host "Installing ClawScript to $InstallDir"
New-Item -ItemType Directory -Force -Path $InstallDir | Out-Null
if (Test-Path "$PSScriptRoot\claw.exe") { Copy-Item -Force "$PSScriptRoot\claw.exe" "$InstallDir\claw.exe" }
if (Test-Path "$PSScriptRoot\claw_tests.exe") { Copy-Item -Force "$PSScriptRoot\claw_tests.exe" "$InstallDir\claw_tests.exe" }
if (Test-Path "$PSScriptRoot\claw_benchmarks.exe") { Copy-Item -Force "$PSScriptRoot\claw_benchmarks.exe" "$InstallDir\claw_benchmarks.exe" }
if (Test-Path "$PSScriptRoot\claw_lsp.exe") { Copy-Item -Force "$PSScriptRoot\claw_lsp.exe" "$InstallDir\claw_lsp.exe" }
if (Test-Path "$PSScriptRoot\claw_api_gen.exe") { Copy-Item -Force "$PSScriptRoot\claw_api_gen.exe" "$InstallDir\claw_api_gen.exe" }
if (Test-Path "$PSScriptRoot\claw-fmt.exe") { Copy-Item -Force "$PSScriptRoot\claw-fmt.exe" "$InstallDir\claw-fmt.exe" }
if (Test-Path "$PSScriptRoot\claw-pm.exe") { Copy-Item -Force "$PSScriptRoot\claw-pm.exe" "$InstallDir\claw-pm.exe" }
$current = [Environment]::GetEnvironmentVariable("Path", "Machine")
if ($current -notlike "*$InstallDir*") {
  [Environment]::SetEnvironmentVariable("Path", $current + ";" + $InstallDir, "Machine")
  Write-Host "Added $InstallDir to PATH"
}
Write-Host "Done"
