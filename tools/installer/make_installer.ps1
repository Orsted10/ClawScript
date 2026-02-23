param(
  [string]$BuildDir = "$PSScriptRoot\..\..\build\bin\Release",
  [string]$OutDir = "$PSScriptRoot\out"
)
New-Item -ItemType Directory -Force -Path $OutDir | Out-Null
function CopyPrefer {
  param([string]$SrcA, [string]$SrcB, [string]$Dst)
  if (Test-Path $SrcA) { Copy-Item -Force $SrcA $Dst }
  elseif (Test-Path $SrcB) { Copy-Item -Force $SrcB $Dst }
}
CopyPrefer "$BuildDir\claw.exe" "$BuildDir\volt.exe" "$OutDir\claw.exe"
CopyPrefer "$BuildDir\claw_tests.exe" "$BuildDir\volt_tests.exe" "$OutDir\claw_tests.exe"
CopyPrefer "$BuildDir\claw_benchmarks.exe" "$BuildDir\volt_benchmarks.exe" "$OutDir\claw_benchmarks.exe"
CopyPrefer "$BuildDir\claw_lsp.exe" "$BuildDir\volt_lsp.exe" "$OutDir\claw_lsp.exe"
CopyPrefer "$BuildDir\claw_api_gen.exe" "$BuildDir\volt_api_gen.exe" "$OutDir\claw_api_gen.exe"
CopyPrefer "$BuildDir\claw-fmt.exe" "$BuildDir\volt-fmt.exe" "$OutDir\claw-fmt.exe"
CopyPrefer "$BuildDir\claw-pm.exe" "$BuildDir\volt-pm.exe" "$OutDir\claw-pm.exe"
Copy-Item -Force "$PSScriptRoot\install.ps1" "$OutDir\install.ps1"
$files = Get-ChildItem $OutDir -Filter *.exe | ForEach-Object { $_.FullName }
$files += "$OutDir\install.ps1"
Compress-Archive -Force -Path $files -DestinationPath "$OutDir\ClawScript-Installer.zip"
Write-Host "Installer archive: $OutDir\ClawScript-Installer.zip"
