# =============================================================================
#  tests\make_expected.ps1 - Sinh dap an chuan (.out) cho cac test co dinh.
#
#  Moi tep .in duoc chay bang CA HAI cai dat:
#      bin\segtree.exe                 (cay phan doan + lazy)
#      bin\segtree.exe --engine=naive  (mang thuong O(N) moi thao tac)
#  Chi khi hai ket qua TRUNG NHAU thi dap an moi duoc ghi ra .out.
#  Nho vay dap an chuan khong bao gio duoc "chung thuc" boi chinh cai dat dang
#  can kiem tra.
#
#  CACH DUNG:  .\tests\make_expected.ps1
# =============================================================================
$ErrorActionPreference = "Stop"
$root = Split-Path -Parent (Split-Path -Parent $MyInvocation.MyCommand.Path)
Set-Location $root

$exe = Join-Path $root "bin\segtree.exe"
if (-not (Test-Path $exe)) { throw "Chua co bin\segtree.exe. Hay chay .\build.ps1 truoc." }

$cases = Get-ChildItem (Join-Path $root "tests\cases") -Filter *.in | Sort-Object Name
if ($cases.Count -eq 0) { throw "Khong tim thay tep .in nao trong tests\cases" }

$bad = 0
foreach ($c in $cases) {
    $outPath = [System.IO.Path]::ChangeExtension($c.FullName, ".out")
    $tmpSeg  = Join-Path $env:TEMP ("seg_"   + $c.BaseName + ".txt")
    $tmpNai  = Join-Path $env:TEMP ("naive_" + $c.BaseName + ".txt")

    & $exe $c.FullName $tmpSeg
    if ($LASTEXITCODE -ne 0) { Write-Host "  LOI khi chay segtree tren $($c.Name)"; $bad++; continue }
    & $exe "--engine=naive" $c.FullName $tmpNai
    if ($LASTEXITCODE -ne 0) { Write-Host "  LOI khi chay naive tren $($c.Name)"; $bad++; continue }

    $a = Get-Content $tmpSeg -Raw
    $b = Get-Content $tmpNai -Raw
    if ($a -ne $b) {
        Write-Host "  KHONG KHOP giua segtree va naive: $($c.Name)" -ForegroundColor Red
        $bad++
    } else {
        Copy-Item $tmpSeg $outPath -Force
        $lines = ($a -split "`n" | Where-Object { $_ -ne "" }).Count
        Write-Host ("  OK  {0,-34} -> {1} dong ket qua" -f $c.Name, $lines)
    }
    Remove-Item $tmpSeg, $tmpNai -ErrorAction SilentlyContinue
}

if ($bad -gt 0) { Write-Host "`nCo $bad tep gap van de." -ForegroundColor Red; exit 1 }
Write-Host "`nDa sinh dap an chuan cho $($cases.Count) test (hai cai dat deu khop)." -ForegroundColor Green
exit 0
