# =============================================================================
#  run_experiments.ps1 - Chay TOAN BO thuc nghiem do thoi gian cua chuyen de 6.
#
#  San pham sinh ra:
#      data\*.txt                  du lieu vao dung cho thuc nghiem
#      results\environment.md      thong tin moi truong chay
#      results\benchmark.csv       do thoi gian trong bo nho (bin\bench.exe)
#      results\end_to_end.csv      do thoi gian tron goi tren tep du lieu that
#      results\results.md          bang ket qua dinh dang Markdown cho bao cao
#
#  CACH DUNG (chay tu thu muc goc du an):
#      .\scripts\run_experiments.ps1                 # bo day du (vai phut)
#      .\scripts\run_experiments.ps1 -Quick          # ban rut gon, chay nhanh
# =============================================================================
param([switch]$Quick)

$ErrorActionPreference = "Stop"
$root = Split-Path -Parent (Split-Path -Parent $MyInvocation.MyCommand.Path)
Set-Location $root

foreach ($e in @("segtree", "bench", "generator")) {
    if (-not (Test-Path (Join-Path $root "bin\$e.exe"))) {
        Write-Host "THIEU bin\$e.exe - hay chay .\scripts\build.ps1 truoc." -ForegroundColor Red
        exit 1
    }
}
New-Item -ItemType Directory -Force -Path "data", "results" | Out-Null

# =============================================================================
#  0. Ghi lai moi truong chay
# =============================================================================
Write-Host "[0/4] Thu thap thong tin moi truong chay..." -ForegroundColor Cyan

$cpu  = Get-CimInstance Win32_Processor | Select-Object -First 1
$os   = Get-CimInstance Win32_OperatingSystem
$ram  = [math]::Round((Get-CimInstance Win32_ComputerSystem).TotalPhysicalMemory / 1GB, 1)
$gpp  = (Get-Command g++ -ErrorAction SilentlyContinue)
if ($gpp) { $gppPath = $gpp.Source } else { $gppPath = "C:\msys64\ucrt64\bin\g++.exe" }
$gppVer = "khong xac dinh"
if (Test-Path $gppPath) {
    $env:PATH = (Split-Path -Parent $gppPath) + ";" + $env:PATH
    $gppVer = (& $gppPath --version | Select-Object -First 1)
}
$stamp = Get-Date -Format "yyyy-MM-dd HH:mm:ss zzz"

$envMd = @"
# Moi truong chay thuc nghiem

| Hang muc | Gia tri |
|---|---|
| Thoi diem do | $stamp |
| He dieu hanh | $($os.Caption) (build $($os.BuildNumber)) |
| CPU | $($cpu.Name.Trim()) |
| So nhan / luong | $($cpu.NumberOfCores) nhan / $($cpu.NumberOfLogicalProcessors) luong |
| Xung nhip co ban | $($cpu.MaxClockSpeed) MHz |
| Bo nho RAM | $ram GB |
| Trinh bien dich | $gppVer |
| Co bien dich | ``-std=c++17 -O2 -DNDEBUG -static`` |
| Dong ho do | ``std::chrono::steady_clock`` |
| Don vi do | mili giay (ms); ``ns/op`` = nano giay tren mot thao tac |
| Cach lay so lieu | moi cau hinh chay lap lai nhieu lan, bao cao gia tri NHO NHAT |

**Ghi chu ve phep do.** Chuoi thao tac duoc sinh truoc va nap san vao bo nho nen
thoi gian do khong lan chi phi sinh so ngau nhien hay nhap/xuat. Gia tri
``checksum`` la tong cua moi ket qua truy van: vua chan trinh toi uu loai bo
vong lap, vua xac nhan cay phan doan va mang thuong cho cung ket qua.
"@
Set-Content -Path "results\environment.md" -Value $envMd -Encoding utf8
Write-Host "      -> results\environment.md"

# =============================================================================
#  1. Sinh du lieu thuc nghiem
# =============================================================================
Write-Host "[1/4] Sinh du lieu vao trong data\ ..." -ForegroundColor Cyan

if ($Quick) {
    $datasets = @(
        @{ File = "small_n1000_q1000.txt";        N = 1000;   Q = 1000;   Mode = "mixed"; Seed = 101 },
        @{ File = "medium_n50000_q50000.txt";     N = 50000;  Q = 50000;  Mode = "mixed"; Seed = 102 }
    )
} else {
    $datasets = @(
        @{ File = "small_n1000_q1000.txt";        N = 1000;    Q = 1000;    Mode = "mixed";  Seed = 101 },
        @{ File = "medium_n50000_q50000.txt";     N = 50000;   Q = 50000;   Mode = "mixed";  Seed = 102 },
        @{ File = "large_n1000000_q1000000.txt";  N = 1000000; Q = 1000000; Mode = "mixed";  Seed = 103 },
        @{ File = "adv_full_n200000_q200000.txt"; N = 200000;  Q = 200000;  Mode = "full";   Seed = 104 },
        @{ File = "adv_point_n200000_q200000.txt";N = 200000;  Q = 200000;  Mode = "point";  Seed = 105 },
        @{ File = "adv_prefix_n200000_q200000.txt";N = 200000; Q = 200000;  Mode = "prefix"; Seed = 106 }
    )
}

foreach ($d in $datasets) {
    $path = Join-Path "data" $d.File
    & .\bin\generator.exe "--n=$($d.N)" "--q=$($d.Q)" "--seed=$($d.Seed)" "--mode=$($d.Mode)" `
                          "--maxval=1000000000" "--maxadd=1000000" "--pupd=0.5" "--out=$path"
    if ($LASTEXITCODE -ne 0) { Write-Host "      LOI khi sinh $path" -ForegroundColor Red; exit 1 }
    $kb = [math]::Round((Get-Item $path).Length / 1KB)
    Write-Host ("      {0,-32} N={1,-8} Q={2,-8} {3} KB" -f $d.File, $d.N, $d.Q, $kb)
}

# =============================================================================
#  2. Do thoi gian trong bo nho
# =============================================================================
Write-Host "[2/4] Do thoi gian trong bo nho (bin\bench.exe)..." -ForegroundColor Cyan
if ($Quick) { $sizes = "1000,10000,100000"; $ops = 50000; $rep = 2 }
else         { $sizes = "1000,10000,100000,1000000"; $ops = 200000; $rep = 5 }

& .\bin\bench.exe "--sizes=$sizes" "--ops=$ops" "--repeat=$rep" "--with-naive" "--out=results\benchmark.csv"
if ($LASTEXITCODE -ne 0) { Write-Host "      LOI khi chay bench" -ForegroundColor Red; exit 1 }

# =============================================================================
#  3. Do thoi gian tron goi tren tep du lieu that
# =============================================================================
Write-Host "[3/4] Do thoi gian tron goi tren tep du lieu (bin\segtree.exe)..." -ForegroundColor Cyan
$e2e = New-Object System.Collections.ArrayList
[void]$e2e.Add("dataset,n,q,engine,wall_ms_min,build_ms_min,ops_ms_min,runs")

$tmpOut   = Join-Path $env:TEMP "e2e_out.txt"
$tmpStats = Join-Path $env:TEMP "e2e_stats.csv"

foreach ($d in $datasets) {
    $path = Join-Path "data" $d.File
    $engines = @("segtree")
    # Loi giai mang thuong chi chay khi N*Q con chap nhan duoc.
    if ([double]$d.N * [double]$d.Q -le 5e9) { $engines += "naive" }

    foreach ($eng in $engines) {
        $runs = 3
        $bestWall  = [double]::MaxValue
        $bestBuild = [double]::MaxValue
        $bestOps   = [double]::MaxValue
        $ok = $true
        for ($i = 0; $i -lt $runs; $i++) {
            $sw = [System.Diagnostics.Stopwatch]::StartNew()
            if ($eng -eq "naive") { & .\bin\segtree.exe "--engine=naive" "--stats=$tmpStats" $path $tmpOut }
            else                  { & .\bin\segtree.exe "--stats=$tmpStats" $path $tmpOut }
            $sw.Stop()
            if ($LASTEXITCODE -ne 0) {
                Write-Host "      LOI: $path / $eng (ma thoat $LASTEXITCODE)" -ForegroundColor Red
                $ok = $false; break
            }
            $bestWall = [Math]::Min($bestWall, $sw.Elapsed.TotalMilliseconds)
            $s = Import-Csv $tmpStats | Select-Object -First 1
            $bestBuild = [Math]::Min($bestBuild, [double]$s.build_ms)
            $bestOps   = [Math]::Min($bestOps,   [double]$s.ops_ms)
        }
        if (-not $ok) { continue }
        [void]$e2e.Add(("{0},{1},{2},{3},{4:F3},{5:F3},{6:F3},{7}" -f `
                        $d.File, $d.N, $d.Q, $eng, $bestWall, $bestBuild, $bestOps, $runs))
        Write-Host ("      {0,-32} {1,-8} tong={2,9:N1} ms  build={3,8:N1} ms  thao-tac={4,9:N1} ms" -f `
                    $d.File, $eng, $bestWall, $bestBuild, $bestOps)
    }
}
Remove-Item $tmpOut, $tmpStats -ErrorAction SilentlyContinue
Set-Content -Path "results\end_to_end.csv" -Value ($e2e -join "`n") -Encoding utf8

# =============================================================================
#  4. Tong hop thanh bang Markdown cho bao cao
# =============================================================================
Write-Host "[4/4] Tong hop results\results.md ..." -ForegroundColor Cyan

$bench = Import-Csv "results\benchmark.csv"
$sb = New-Object System.Text.StringBuilder
[void]$sb.AppendLine("# Ket qua thuc nghiem - Chuyen de 6: Segment Tree + Lazy Propagation")
[void]$sb.AppendLine("")
[void]$sb.AppendLine("Sinh tu dong boi ``run_experiments.ps1`` luc $stamp.")
[void]$sb.AppendLine("Thong tin moi truong chay: xem [environment.md](environment.md).")
[void]$sb.AppendLine("")
[void]$sb.AppendLine("Ky hieu: **N** = so phan tu, **Q** = so thao tac, **ns/op** = nano giay")
[void]$sb.AppendLine("trung binh cho mot thao tac. Moi o la gia tri NHO NHAT qua $rep lan chay.")
[void]$sb.AppendLine("")

# --- Bang 1: thoi gian theo N, tai hon hop ---
[void]$sb.AppendLine("## Bang 1. Thoi gian theo kich thuoc N (tai hon hop 50% cap nhat / 50% truy van)")
[void]$sb.AppendLine("")
[void]$sb.AppendLine("| N | Q | Segment Tree (ms) | ns/op | Mang thuong (ms) | ns/op | Ty le tang toc |")
[void]$sb.AppendLine("|---:|---:|---:|---:|---:|---:|---:|")
foreach ($n in ($bench | Where-Object { $_.workload -eq "mixed" } | ForEach-Object { [int]$_.n } | Sort-Object -Unique)) {
    $s = $bench | Where-Object { $_.workload -eq "mixed" -and [int]$_.n -eq $n -and $_.engine -eq "segtree" } | Select-Object -First 1
    $v = $bench | Where-Object { $_.workload -eq "mixed" -and [int]$_.n -eq $n -and $_.engine -eq "naive"   } | Select-Object -First 1
    if ($v) {
        $ratio = "{0:N1}x" -f ([double]$v.ops_ms_min / [double]$s.ops_ms_min)
        [void]$sb.AppendLine("| $n | $($s.q) | $($s.ops_ms_min) | $($s.ns_per_op) | $($v.ops_ms_min) | $($v.ns_per_op) | $ratio |")
    } else {
        [void]$sb.AppendLine("| $n | $($s.q) | $($s.ops_ms_min) | $($s.ns_per_op) | khong do (qua cham) | - | - |")
    }
}
[void]$sb.AppendLine("")

# --- Bang 2: thoi gian theo kieu tai ---
[void]$sb.AppendLine("## Bang 2. Thoi gian cua Segment Tree theo KIEU TAI")
[void]$sb.AppendLine("")
[void]$sb.AppendLine("| N | mixed (ns/op) | update (ns/op) | query (ns/op) | point (ns/op) | full (ns/op) |")
[void]$sb.AppendLine("|---:|---:|---:|---:|---:|---:|")
foreach ($n in ($bench | ForEach-Object { [int]$_.n } | Sort-Object -Unique)) {
    $row = "| $n "
    foreach ($w in @("mixed", "update", "query", "point", "full")) {
        $r = $bench | Where-Object { $_.engine -eq "segtree" -and $_.workload -eq $w -and [int]$_.n -eq $n } | Select-Object -First 1
        if ($r) { $row += "| $($r.ns_per_op) " } else { $row += "| - " }
    }
    [void]$sb.AppendLine($row + "|")
}
[void]$sb.AppendLine("")
[void]$sb.AppendLine("Kieu tai ``full`` (moi thao tac phu toan doan [1, N]) la truong hop lazy co loi")
[void]$sb.AppendLine("nhat: cap nhat dung ngay o nut goc nen chi ton O(1).")
[void]$sb.AppendLine("")

# --- Bang 3: dung cay va bo nho ---
[void]$sb.AppendLine("## Bang 3. Thoi gian dung cay va bo nho su dung")
[void]$sb.AppendLine("")
[void]$sb.AppendLine("| N | Thoi gian build (ms) | Bo nho cay (MB) | Byte / phan tu |")
[void]$sb.AppendLine("|---:|---:|---:|---:|")
foreach ($n in ($bench | ForEach-Object { [int]$_.n } | Sort-Object -Unique)) {
    $r = $bench | Where-Object { $_.engine -eq "segtree" -and $_.workload -eq "mixed" -and [int]$_.n -eq $n } | Select-Object -First 1
    if ($r) {
        $mb = "{0:N2}" -f ([double]$r.tree_bytes / 1MB)
        $bpe = "{0:N1}" -f ([double]$r.tree_bytes / [double]$n)
        [void]$sb.AppendLine("| $n | $($r.build_ms) | $mb | $bpe |")
    }
}
[void]$sb.AppendLine("")
[void]$sb.AppendLine("Cay cap phat 4N o cho gia tri gop va 4N o cho the lazy, moi o 8 byte,")
[void]$sb.AppendLine("nen ly thuyet la 64 byte cho moi phan tu cua mang goc.")
[void]$sb.AppendLine("")

# --- Bang 4: end-to-end ---
[void]$sb.AppendLine("## Bang 4. Thoi gian tron goi tren tep du lieu that")
[void]$sb.AppendLine("")
[void]$sb.AppendLine("``Tong`` bao gom khoi dong tien trinh, doc tep, xu ly va ghi tep.")
[void]$sb.AppendLine("``Build`` la thoi gian dung cay, ``Thao tac`` la thoi gian thuc hien Q thao tac.")
[void]$sb.AppendLine("")
[void]$sb.AppendLine("| Tep du lieu | N | Q | Cai dat | Tong (ms) | Build (ms) | Thao tac (ms) |")
[void]$sb.AppendLine("|---|---:|---:|---|---:|---:|---:|")
foreach ($line in ($e2e | Select-Object -Skip 1)) {
    $p = $line -split ","
    [void]$sb.AppendLine("| $($p[0]) | $($p[1]) | $($p[2]) | $($p[3]) | $($p[4]) | $($p[5]) | $($p[6]) |")
}
[void]$sb.AppendLine("")
[void]$sb.AppendLine("## Nhan xet")
[void]$sb.AppendLine("")
[void]$sb.AppendLine("1. Thoi gian moi thao tac cua Segment Tree tang **tuyen tinh theo log N**:")
[void]$sb.AppendLine("   N tang 10 lan (log2 N tang khoang 3.32) thi ns/op chi tang mot luong nho,")
[void]$sb.AppendLine("   hoan toan khac voi mang thuong tang gan dung 10 lan.")
[void]$sb.AppendLine("2. Phan tang ns/op o N lon con den tu **hieu ung bo nho dem**: cay 1 trieu")
[void]$sb.AppendLine("   phan tu chiem khoang 64 MB, vuot xa bo nho dem cap ba cua CPU.")
[void]$sb.AppendLine("3. O kieu tai ``full``, cap nhat dung ngay tai nut goc nen thoi gian gan nhu")
[void]$sb.AppendLine("   khong phu thuoc N - day chinh la loi ich cua lazy propagation.")
[void]$sb.AppendLine("4. Ty le tang toc so voi mang thuong lon dan theo N, dung nhu du doan cua")
[void]$sb.AppendLine("   phan tich do phuc tap O(N) so voi O(log N).")

Set-Content -Path "results\results.md" -Value $sb.ToString() -Encoding utf8
Write-Host "      -> results\results.md"
Write-Host ""
Write-Host "HOAN TAT THUC NGHIEM." -ForegroundColor Green
Write-Host "  results\environment.md   thong tin moi truong"
Write-Host "  results\benchmark.csv    so lieu tho (trong bo nho)"
Write-Host "  results\end_to_end.csv   so lieu tho (tron goi)"
Write-Host "  results\results.md       bang ket qua cho bao cao"
exit 0
