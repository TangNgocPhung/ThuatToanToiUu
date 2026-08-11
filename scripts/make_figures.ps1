# =============================================================================
#  make_figures.ps1 - Sinh hinh minh hoa (SVG) tu results\benchmark.csv.
#
#  San pham (dat trong report\figures\):
#      fig1_scaling.svg      ns/op theo N, thang loga hai truc: Segment Tree
#                            so voi mang thuong.
#      fig2_logn.svg         ns/op theo log2(N), truc tung tuyen tinh - dung de
#                            kiem chung truc quan gia thuyet O(log N).
#      fig3_workloads.svg    ns/op theo tung kieu tai, bieu do cot nhom.
#
#  Tep SVG la van ban thuan, chen truc tiep vao Word/LaTeX/Markdown deu duoc.
#
#  CACH DUNG (chay tu thu muc goc du an):  .\scripts\make_figures.ps1
# =============================================================================
$ErrorActionPreference = "Stop"
$root = Split-Path -Parent (Split-Path -Parent $MyInvocation.MyCommand.Path)
Set-Location $root

$csvPath = "results\benchmark.csv"
if (-not (Test-Path $csvPath)) { throw "Chua co $csvPath. Hay chay .\scripts\run_experiments.ps1 truoc." }
New-Item -ItemType Directory -Force -Path "report\figures" | Out-Null

$rows = Import-Csv $csvPath
$W = 760; $H = 460
$ML = 90; $MR = 210; $MT = 50; $MB = 70   # le trai / phai / tren / duoi
$PW = $W - $ML - $MR
$PH = $H - $MT - $MB

function Esc([string]$s) { $s -replace '&', '&amp;' -replace '<', '&lt;' -replace '>', '&gt;' }

function New-Svg([string]$title) {
    $sb = New-Object System.Text.StringBuilder
    [void]$sb.AppendLine("<?xml version='1.0' encoding='UTF-8'?>")
    [void]$sb.AppendLine("<svg xmlns='http://www.w3.org/2000/svg' width='$W' height='$H' viewBox='0 0 $W $H' font-family='Segoe UI, Arial, sans-serif'>")
    [void]$sb.AppendLine("<rect width='$W' height='$H' fill='#ffffff'/>")
    [void]$sb.AppendLine("<text x='$($W/2)' y='28' text-anchor='middle' font-size='17' font-weight='600' fill='#111'>$(Esc $title)</text>")
    return $sb
}

function Add-Axes($sb, [string]$xlab, [string]$ylab) {
    $x0 = $ML; $y0 = $MT + $PH
    [void]$sb.AppendLine("<line x1='$x0' y1='$MT' x2='$x0' y2='$y0' stroke='#333' stroke-width='1.5'/>")
    [void]$sb.AppendLine("<line x1='$x0' y1='$y0' x2='$($ML+$PW)' y2='$y0' stroke='#333' stroke-width='1.5'/>")
    [void]$sb.AppendLine("<text x='$($ML+$PW/2)' y='$($H-22)' text-anchor='middle' font-size='13' fill='#333'>$(Esc $xlab)</text>")
    [void]$sb.AppendLine("<text x='22' y='$($MT+$PH/2)' text-anchor='middle' font-size='13' fill='#333' transform='rotate(-90 22 $($MT+$PH/2))'>$(Esc $ylab)</text>")
}

function Add-Legend($sb, $items) {   # items: mang @{Label=..; Color=..}
    $x = $ML + $PW + 24
    $y = $MT + 10
    foreach ($it in $items) {
        [void]$sb.AppendLine("<rect x='$x' y='$($y-9)' width='13' height='13' fill='$($it.Color)' rx='2'/>")
        [void]$sb.AppendLine("<text x='$($x+19)' y='$($y+2)' font-size='12' fill='#222'>$(Esc $it.Label)</text>")
        $y += 22
    }
}

# =============================================================================
#  Hinh 1 - ns/op theo N (thang loga hai truc)
# =============================================================================
$seg   = $rows | Where-Object { $_.engine -eq "segtree" -and $_.workload -eq "mixed" } | Sort-Object { [int]$_.n }
$naive = $rows | Where-Object { $_.engine -eq "naive"   -and $_.workload -eq "mixed" } | Sort-Object { [int]$_.n }
if ($seg.Count -lt 2) { throw "Khong du diem du lieu de ve hinh." }

$allNs = @($seg | ForEach-Object { [double]$_.ns_per_op }) + @($naive | ForEach-Object { [double]$_.ns_per_op })
$logNmin = [Math]::Log10([double]($seg[0].n)); $logNmax = [Math]::Log10([double]($seg[-1].n))
$logYmin = [Math]::Floor([Math]::Log10(($allNs | Measure-Object -Minimum).Minimum))
$logYmax = [Math]::Ceiling([Math]::Log10(($allNs | Measure-Object -Maximum).Maximum))

function PX([double]$n) { $ML + $PW * ([Math]::Log10($n) - $logNmin) / ($logNmax - $logNmin) }
function PY([double]$v) { $MT + $PH - $PH * ([Math]::Log10($v) - $logYmin) / ($logYmax - $logYmin) }

$sb = New-Svg "Thoi gian trung binh moi thao tac theo kich thuoc N (tai hon hop)"
for ($e = $logYmin; $e -le $logYmax; $e++) {
    $y = PY ([Math]::Pow(10, $e))
    [void]$sb.AppendLine("<line x1='$ML' y1='$y' x2='$($ML+$PW)' y2='$y' stroke='#e8e8e8'/>")
    [void]$sb.AppendLine("<text x='$($ML-10)' y='$($y+4)' text-anchor='end' font-size='11' fill='#555'>1e$e</text>")
}
foreach ($r in $seg) {
    $x = PX ([double]$r.n)
    [void]$sb.AppendLine("<line x1='$x' y1='$MT' x2='$x' y2='$($MT+$PH)' stroke='#f2f2f2'/>")
    [void]$sb.AppendLine("<text x='$x' y='$($MT+$PH+20)' text-anchor='middle' font-size='11' fill='#555'>$($r.n)</text>")
}
Add-Axes $sb "N (thang loga)" "ns moi thao tac (thang loga)"

foreach ($pair in @(@{Data=$seg; Color='#1f77b4'}, @{Data=$naive; Color='#d62728'})) {
    if ($pair.Data.Count -lt 1) { continue }
    $pts = ($pair.Data | ForEach-Object { "$(PX ([double]$_.n)),$(PY ([double]$_.ns_per_op))" }) -join " "
    [void]$sb.AppendLine("<polyline points='$pts' fill='none' stroke='$($pair.Color)' stroke-width='2.5'/>")
    foreach ($r in $pair.Data) {
        [void]$sb.AppendLine("<circle cx='$(PX ([double]$r.n))' cy='$(PY ([double]$r.ns_per_op))' r='4.5' fill='$($pair.Color)'/>")
    }
}
Add-Legend $sb @(@{Label="Segment Tree O(log N)"; Color='#1f77b4'}, @{Label="Mang thuong O(N)"; Color='#d62728'})
[void]$sb.AppendLine("</svg>")
Set-Content "report\figures\fig1_scaling.svg" $sb.ToString() -Encoding utf8
Write-Host "  -> report\figures\fig1_scaling.svg"

# =============================================================================
#  Hinh 2 - ns/op theo log2(N), truc tung TUYEN TINH
#           Neu do phuc tap dung la O(log N) thi cac diem phai gan mot duong thang.
# =============================================================================
$xs = $seg | ForEach-Object { [Math]::Log([double]$_.n, 2) }
$ys = $seg | ForEach-Object { [double]$_.ns_per_op }
$xmin = ($xs | Measure-Object -Minimum).Minimum; $xmax = ($xs | Measure-Object -Maximum).Maximum
$ymax = ($ys | Measure-Object -Maximum).Maximum * 1.15

function QX([double]$v) { $ML + $PW * ($v - $xmin) / ($xmax - $xmin) }
function QY([double]$v) { $MT + $PH - $PH * $v / $ymax }

$sb = New-Svg "Kiem chung O(log N): ns/op theo log2(N), truc tung tuyen tinh"
for ($i = 0; $i -le 5; $i++) {
    $v = $ymax * $i / 5
    $y = QY $v
    [void]$sb.AppendLine("<line x1='$ML' y1='$y' x2='$($ML+$PW)' y2='$y' stroke='#e8e8e8'/>")
    [void]$sb.AppendLine("<text x='$($ML-10)' y='$($y+4)' text-anchor='end' font-size='11' fill='#555'>$([Math]::Round($v))</text>")
}
for ($i = 0; $i -lt $seg.Count; $i++) {
    $x = QX $xs[$i]
    [void]$sb.AppendLine("<line x1='$x' y1='$MT' x2='$x' y2='$($MT+$PH)' stroke='#f2f2f2'/>")
    [void]$sb.AppendLine("<text x='$x' y='$($MT+$PH+20)' text-anchor='middle' font-size='11' fill='#555'>$([Math]::Round($xs[$i],1))</text>")
    [void]$sb.AppendLine("<text x='$x' y='$($MT+$PH+36)' text-anchor='middle' font-size='10' fill='#999'>N=$($seg[$i].n)</text>")
}
Add-Axes $sb "log2(N)" "ns moi thao tac"

# Duong hoi quy tuyen tinh binh phuong toi thieu
$k = $xs.Count
$sx = ($xs | Measure-Object -Sum).Sum; $sy = ($ys | Measure-Object -Sum).Sum
$sxx = 0.0; $sxy = 0.0
for ($i = 0; $i -lt $k; $i++) { $sxx += $xs[$i] * $xs[$i]; $sxy += $xs[$i] * $ys[$i] }
$slope = ($k * $sxy - $sx * $sy) / ($k * $sxx - $sx * $sx)
$intercept = ($sy - $slope * $sx) / $k
# He so xac dinh R^2
$meanY = $sy / $k; $ssTot = 0.0; $ssRes = 0.0
for ($i = 0; $i -lt $k; $i++) {
    $pred = $slope * $xs[$i] + $intercept
    $ssTot += [Math]::Pow($ys[$i] - $meanY, 2); $ssRes += [Math]::Pow($ys[$i] - $pred, 2)
}
$r2 = 1 - $ssRes / $ssTot
[void]$sb.AppendLine("<line x1='$(QX $xmin)' y1='$(QY ($slope*$xmin+$intercept))' x2='$(QX $xmax)' y2='$(QY ($slope*$xmax+$intercept))' stroke='#999' stroke-width='1.5' stroke-dasharray='6 4'/>")
$pts = @(); for ($i = 0; $i -lt $k; $i++) { $pts += "$(QX $xs[$i]),$(QY $ys[$i])" }
[void]$sb.AppendLine("<polyline points='$($pts -join ' ')' fill='none' stroke='#1f77b4' stroke-width='2.5'/>")
for ($i = 0; $i -lt $k; $i++) { [void]$sb.AppendLine("<circle cx='$(QX $xs[$i])' cy='$(QY $ys[$i])' r='4.5' fill='#1f77b4'/>") }
Add-Legend $sb @(@{Label="Do duoc"; Color='#1f77b4'}, @{Label="Hoi quy tuyen tinh"; Color='#999999'})
if ($intercept -lt 0) { $sign = "-" } else { $sign = "+" }
$fit = "y = {0:F1}*log2(N) {1} {2:F1}" -f $slope, $sign, [Math]::Abs($intercept)
[void]$sb.AppendLine("<text x='$($ML+$PW+24)' y='$($MT+70)' font-size='11' fill='#333'>$(Esc $fit)</text>")
[void]$sb.AppendLine("<text x='$($ML+$PW+24)' y='$($MT+88)' font-size='11' fill='#333'>R^2 = $([Math]::Round($r2,4))</text>")
[void]$sb.AppendLine("</svg>")
Set-Content "report\figures\fig2_logn.svg" $sb.ToString() -Encoding utf8
Write-Host "  -> report\figures\fig2_logn.svg  (R^2 = $([Math]::Round($r2,4)))"

# =============================================================================
#  Hinh 3 - ns/op theo tung kieu tai (bieu do cot nhom)
# =============================================================================
$loads = @("mixed", "update", "query", "point", "full")
$colors = @{ mixed='#1f77b4'; update='#ff7f0e'; query='#2ca02c'; point='#9467bd'; full='#8c564b' }
$sizesN = $rows | Where-Object { $_.engine -eq "segtree" } | ForEach-Object { [int]$_.n } | Sort-Object -Unique
$maxNs = ($rows | Where-Object { $_.engine -eq "segtree" } | ForEach-Object { [double]$_.ns_per_op } | Measure-Object -Maximum).Maximum * 1.12

$sb = New-Svg "Thoi gian moi thao tac theo KIEU TAI (Segment Tree)"
for ($i = 0; $i -le 4; $i++) {
    $v = $maxNs * $i / 4
    $y = $MT + $PH - $PH * $i / 4
    [void]$sb.AppendLine("<line x1='$ML' y1='$y' x2='$($ML+$PW)' y2='$y' stroke='#e8e8e8'/>")
    [void]$sb.AppendLine("<text x='$($ML-10)' y='$($y+4)' text-anchor='end' font-size='11' fill='#555'>$([Math]::Round($v))</text>")
}
Add-Axes $sb "N" "ns moi thao tac"
$groupW = $PW / $sizesN.Count
$barW = ($groupW * 0.72) / $loads.Count
for ($g = 0; $g -lt $sizesN.Count; $g++) {
    $n = $sizesN[$g]
    $gx = $ML + $groupW * $g + $groupW * 0.14
    for ($b = 0; $b -lt $loads.Count; $b++) {
        $r = $rows | Where-Object { $_.engine -eq "segtree" -and $_.workload -eq $loads[$b] -and [int]$_.n -eq $n } | Select-Object -First 1
        if (-not $r) { continue }
        $v = [double]$r.ns_per_op
        $h = $PH * $v / $maxNs
        $x = $gx + $barW * $b
        $y = $MT + $PH - $h
        [void]$sb.AppendLine("<rect x='$([Math]::Round($x,1))' y='$([Math]::Round($y,1))' width='$([Math]::Round($barW-1.5,1))' height='$([Math]::Round($h,1))' fill='$($colors[$loads[$b]])' rx='1.5'/>")
    }
    [void]$sb.AppendLine("<text x='$($ML + $groupW*$g + $groupW/2)' y='$($MT+$PH+20)' text-anchor='middle' font-size='11' fill='#555'>$n</text>")
}
Add-Legend $sb ($loads | ForEach-Object { @{ Label = $_; Color = $colors[$_] } })
[void]$sb.AppendLine("<text x='$($ML+$PW+24)' y='$($MT+150)' font-size='11' fill='#333'>Cot 'full' gan nhu khong</text>")
[void]$sb.AppendLine("<text x='$($ML+$PW+24)' y='$($MT+166)' font-size='11' fill='#333'>tang theo N: cap nhat</text>")
[void]$sb.AppendLine("<text x='$($ML+$PW+24)' y='$($MT+182)' font-size='11' fill='#333'>dung ngay o nut goc.</text>")
[void]$sb.AppendLine("</svg>")
Set-Content "report\figures\fig3_workloads.svg" $sb.ToString() -Encoding utf8
Write-Host "  -> report\figures\fig3_workloads.svg"

Write-Host ""
Write-Host "Da sinh 3 hinh minh hoa trong report\figures\" -ForegroundColor Green
