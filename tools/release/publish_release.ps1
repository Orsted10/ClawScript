param(
  [string]$Tag,
  [string]$Title = "VoltScript Release",
  [string]$Notes = "Installer and binaries",
  [string]$AssetsDir = "$PSScriptRoot\..\installer\out"
)
if (-not (Get-Command gh -ErrorAction SilentlyContinue)) {
  Write-Error "GitHub CLI 'gh' not found"
  exit 1
}
gh release create $Tag "$AssetsDir\VoltScript-Installer.zip" --title $Title --notes $Notes
Write-Host "Published release $Tag"

