# =============================================================================
#  run_tests.ps1 - Chay TOAN BO bo kiem thu cua chuyen de 6.
#
#  Gom bon buoc:
#      1. Kiem thu don vi (co ban / bien / doi khang / ngau nhien / mo rong)
#      2. Kiem thu don vi ban co kiem tra tran 64-bit (ST_CHECK_OVERFLOW)
#      3. Chay 12 test co dinh trong tests\cases va so voi dap an chuan
#      4. Stress test ngau nhien doi chieu voi loi giai mang thuong
#
#  CACH DUNG (chay tu thu muc goc du an):
#      .\scripts\run_tests.ps1                       # bo day du
#      .\scripts\run_tests.ps1 -StressRounds 20000   # stress test lau hon
#      .\scripts\run_tests.ps1 -SkipStress           # bo qua stress test
#
#  Ma thoat: 0 = tat ca dat, 1 = co buoc that bai.
#  Nhat ky duoc ghi vao results\test_log.txt
# =============================================================================
param(
    [int]$StressRounds = 3000,
    [switch]$SkipStress
)

$ErrorActionPreference = "Stop"
$root = Split-Path -Parent (Split-Path -Parent $MyInvocation.MyCommand.Path)
Set-Location $root

$log = Join-Path $root "results\test_log.txt"
New-Item -ItemType Directory -Force -Path (Join-Path $root "results") | Out-Null
$transcript = New-Object System.Text.StringBuilder

function Say([string]$msg, [string]$color = "Gray") {
    Write-Host $msg -ForegroundColor $color
    [void]$transcript.AppendLine($msg)
}

$stamp = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
Say "============================================================"
Say " BO KIEM THU - Chuyen de 6: Segment Tree + Lazy Propagation"
Say " Thoi diem chay: $stamp"
Say " May: $env:COMPUTERNAME / $([System.Environment]::OSVersion.VersionString)"
Say "============================================================"

foreach ($e in @("segtree", "unit_tests", "unit_tests_checked", "stress_test")) {
    if (-not (Test-Path (Join-Path $root "bin\$e.exe"))) {
        Say "THIEU bin\$e.exe - hay chay .\scripts\build.ps1 truoc." "Red"
        exit 1
    }
}

$failures = 0

# --- 1. Kiem thu don vi -------------------------------------------------------
Say ""
Say "[1/4] Kiem thu don vi (ban toi uu)" "Cyan"
$out = & .\bin\unit_tests.exe
$code = $LASTEXITCODE
$out | ForEach-Object { [void]$transcript.AppendLine($_) }
$summary = $out | Select-String "Tong ket:" | Select-Object -Last 1
if ($code -eq 0) { Say "      $summary" "Green" } else { Say "      THAT BAI: $summary" "Red"; $out | Select-String "\[FAIL\]" | ForEach-Object { Say "      $_" "Red" }; $failures++ }

# --- 2. Kiem thu don vi + kiem tra tran 64-bit --------------------------------
Say ""
Say "[2/4] Kiem thu don vi (ban co ST_CHECK_OVERFLOW + assert)" "Cyan"
$out = & .\bin\unit_tests_checked.exe
$code = $LASTEXITCODE
$out | ForEach-Object { [void]$transcript.AppendLine($_) }
$summary = $out | Select-String "Tong ket:" | Select-Object -Last 1
if ($code -eq 0) { Say "      $summary" "Green" } else { Say "      THAT BAI: $summary" "Red"; $failures++ }

# --- 3. Test co dinh ----------------------------------------------------------
Say ""
Say "[3/4] Test co dinh trong tests\cases" "Cyan"
$cases = Get-ChildItem (Join-Path $root "tests\cases") -Filter *.in | Sort-Object Name
$casePass = 0; $caseFail = 0
foreach ($c in $cases) {
    $expected = [System.IO.Path]::ChangeExtension($c.FullName, ".out")
    if (-not (Test-Path $expected)) {
        Say ("      [BO QUA] {0}: chua co dap an chuan (.out)" -f $c.Name) "Yellow"
        continue
    }
    $tmp = Join-Path $env:TEMP ("run_" + $c.BaseName + ".txt")
    & .\bin\segtree.exe $c.FullName $tmp
    if ($LASTEXITCODE -ne 0) {
        Say ("      [FAIL] {0}: chuong trinh thoat voi ma {1}" -f $c.Name, $LASTEXITCODE) "Red"
        $caseFail++; continue
    }
    $got  = (Get-Content $tmp  -Raw) -replace "`r`n", "`n"
    $want = (Get-Content $expected -Raw) -replace "`r`n", "`n"
    if ($got -eq $want) {
        Say ("      [ OK ] {0}" -f $c.Name)
        $casePass++
    } else {
        Say ("      [FAIL] {0}: ket qua khac dap an chuan" -f $c.Name) "Red"
        $gl = $got -split "`n"; $wl = $want -split "`n"
        for ($i = 0; $i -lt [Math]::Max($gl.Count, $wl.Count); $i++) {
            if ($gl[$i] -ne $wl[$i]) {
                Say ("             dong {0}: nhan '{1}', mong doi '{2}'" -f ($i + 1), $gl[$i], $wl[$i]) "Red"
                break
            }
        }
        $caseFail++
    }
    Remove-Item $tmp -ErrorAction SilentlyContinue
}
Say ("      Ket qua: {0} dat, {1} hong tren tong {2} test co dinh" -f $casePass, $caseFail, $cases.Count) $(if ($caseFail -eq 0) { "Green" } else { "Red" })
if ($caseFail -gt 0) { $failures++ }

# --- 4. Stress test -----------------------------------------------------------
Say ""
if ($SkipStress) {
    Say "[4/4] Stress test: BO QUA (-SkipStress)" "Yellow"
} else {
    Say "[4/4] Stress test ngau nhien doi chieu ($StressRounds vong)" "Cyan"
    $out = & .\bin\stress_test.exe "--rounds=$StressRounds" "--maxn=200" "--ops=500" "--seed=1" "--quiet"
    $code = $LASTEXITCODE
    $out | ForEach-Object { [void]$transcript.AppendLine($_) }
    if ($code -eq 0) {
        Say ("      " + ($out | Select-Object -Last 1)) "Green"
    } else {
        Say "      THAT BAI - kich ban loi da duoc ghi ra stress_fail.in" "Red"
        $out | ForEach-Object { Say "      $_" "Red" }
        $failures++
    }
}

# --- Tong ket -----------------------------------------------------------------
Say ""
Say "============================================================"
if ($failures -eq 0) {
    Say " KET QUA CHUNG: TAT CA CAC BUOC DEU DAT" "Green"
} else {
    Say " KET QUA CHUNG: CO $failures BUOC THAT BAI" "Red"
}
Say "============================================================"

Set-Content -Path $log -Value $transcript.ToString() -Encoding utf8
Write-Host "Nhat ky day du: results\test_log.txt"
exit $(if ($failures -eq 0) { 0 } else { 1 })
