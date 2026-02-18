Param()
$root = Get-Location
$exe = Join-Path $root "build\\bin\\Release\\volt-fmt.exe"
if (!(Test-Path $exe)) {
  $exe = Join-Path $root "build\\bin\\Debug\\volt-fmt.exe"
}
if (!(Test-Path $exe)) {
  Write-Host "volt-fmt not found; skipping format check"
  exit 0
}
& $exe --check --root="$root"
if ($LASTEXITCODE -ne 0) {
  Write-Host "Formatting changes required. Run: volt-fmt --write"
  exit 1
}
exit 0
