Param()
$root = Get-Location
$exe = Join-Path $root "build\\bin\\Release\\claw-fmt.exe"
if (!(Test-Path $exe)) {
  $exe = Join-Path $root "build\\bin\\Debug\\claw-fmt.exe"
}
if (!(Test-Path $exe)) {
  Write-Host "claw-fmt not found; skipping format check"
  exit 0
}
& $exe --check --root="$root"
if ($LASTEXITCODE -ne 0) {
  Write-Host "Formatting changes required. Run: claw-fmt --write"
  exit 1
}
exit 0
