Param()
$ErrorActionPreference = "Stop"
function Time-Command($cmd, $args) {
  $sw = [System.Diagnostics.Stopwatch]::StartNew()
  try {
    & $cmd $args
  } finally {
    $sw.Stop()
  }
  return [math]::Round($sw.Elapsed.TotalMilliseconds, 3)
}
$root = Get-Location
$volt = Join-Path $root "build\\bin\\Release\\claw.exe"
if (!(Test-Path $volt)) { $volt = Join-Path $root "build\\bin\\claw" }
$voltScript = Join-Path $root "tools\\bench\\mandelbrot.claw"
$py = (Get-Command python -ErrorAction SilentlyContinue)
$lua = (Get-Command luajit -ErrorAction SilentlyContinue)
Write-Host "Benchmark: mandelbrot (80x40, 100 iter)"
if (Test-Path $volt) {
  $ms = Time-Command $volt $voltScript
  Write-Host ("ClawScript: {0} ms" -f $ms)
} else {
  Write-Host "ClawScript binary not found"
}
if ($py) {
  $ms = Time-Command $py.Path (Join-Path $root "tools\\bench\\mandelbrot.py")
  Write-Host ("Python:     {0} ms" -f $ms)
} else {
  Write-Host "Python not found"
}
if ($lua) {
  $ms = Time-Command $lua.Path (Join-Path $root "tools\\bench\\mandelbrot.lua")
  Write-Host ("LuaJIT:     {0} ms" -f $ms)
} else {
  Write-Host "LuaJIT not found"
}
