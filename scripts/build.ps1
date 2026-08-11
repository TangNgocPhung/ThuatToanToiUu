# =============================================================================
#  build.ps1 - Bien dich toan bo chuong trinh cua chuyen de 6 (Windows).
#
#  CACH DUNG (chay tu thu muc goc du an):
#      .\scripts\build.ps1                    # bien dich ban toi uu (-O2)
#      .\scripts\build.ps1 -Debug             # them -g -fsanitize=undefined + kiem tra tran
#      .\scripts\build.ps1 -Compiler "C:\path\to\g++.exe"
#
#  Ket qua dat trong thu muc .\bin\
#  Script tu do tim g++; khong co duong dan ca nhan nao duoc gan cung.
# =============================================================================
param(
    [switch]$Debug,
    [string]$Compiler = ""
)

$ErrorActionPreference = "Stop"
$root = Split-Path -Parent (Split-Path -Parent $MyInvocation.MyCommand.Path)
Set-Location $root

# --- Tim trinh bien dich -----------------------------------------------------
function Find-Compiler {
    param([string]$Explicit)
    if ($Explicit -ne "") {
        if (Test-Path $Explicit) { return $Explicit }
        throw "Khong tim thay trinh bien dich tai: $Explicit"
    }
    $c = Get-Command g++ -ErrorAction SilentlyContinue
    if ($c) { return $c.Source }
    $candidates = @(
        "C:\msys64\ucrt64\bin\g++.exe",
        "C:\msys64\mingw64\bin\g++.exe",
        "C:\mingw64\bin\g++.exe",
        "C:\MinGW\bin\g++.exe",
        "C:\TDM-GCC-64\bin\g++.exe"
    )
    foreach ($p in $candidates) { if (Test-Path $p) { return $p } }
    throw "Khong tim thay g++. Cai MSYS2/MinGW-w64 hoac truyen -Compiler <duong-dan>."
}

$gpp = Find-Compiler -Explicit $Compiler

# Tren Windows, cc1plus.exe nam o thu muc khac g++.exe va can cac DLL trong
# thu muc bin cua trinh bien dich. Neu thu muc do khong co trong PATH thi moi
# lenh bien dich deu that bai voi ma loi 1 ma khong in ra thong bao nao.
$gppDir = Split-Path -Parent $gpp
if (($env:PATH -split ';') -notcontains $gppDir) { $env:PATH = "$gppDir;$env:PATH" }

Write-Host "Trinh bien dich : $gpp"
& $gpp --version | Select-Object -First 1 | ForEach-Object { Write-Host "Phien ban       : $_" }

# --- Co bien dich ------------------------------------------------------------
#
#  LIEN KET TINH (-static): tren Windows rat de xay ra xung dot DLL. Vi du,
#  Git for Windows dat mot ban libstdc++-6.dll / libgcc_s_seh-1.dll KHAC trong
#  C:\Program Files\Git\mingw64\bin va thu muc do thuong dung truoc trong PATH.
#  Chuong trinh se nap nham DLL do va do vo (0xC0000005) ngay khi tao mot doi
#  tuong std::ofstream. Lien ket tinh lam cac tep .exe doc lap hoan toan, chay
#  duoc tren may khac ma khong can cai MSYS2.
$common = @("-std=c++17", "-Wall", "-Wextra", "-Wshadow", "-Wconversion", "-pedantic",
            "-Isrc", "-static", "-static-libgcc", "-static-libstdc++")
if ($Debug) {
    $flags = $common + @("-O1", "-g", "-DST_CHECK_OVERFLOW", "-fno-omit-frame-pointer")
    Write-Host "Che do          : DEBUG (co kiem tra tran 64-bit va assert)"
} else {
    $flags = $common + @("-O2", "-DNDEBUG")
    Write-Host "Che do          : RELEASE (-O2 -DNDEBUG)"
}

New-Item -ItemType Directory -Force -Path "bin" | Out-Null

$targets = @(
    @{ Name = "segtree";      Src = "src\main.cpp" },
    @{ Name = "bench";        Src = "src\bench.cpp" },
    @{ Name = "generator";    Src = "tests\generator.cpp" },
    @{ Name = "stress_test";  Src = "tests\stress_test.cpp" },
    @{ Name = "unit_tests";   Src = "tests\unit_tests.cpp" }
)

$fail = $false
foreach ($t in $targets) {
    $out = "bin\$($t.Name).exe"
    Write-Host ""
    Write-Host ">> $($t.Src)  ->  $out"
    & $gpp $flags $t.Src -o $out
    if ($LASTEXITCODE -ne 0) { Write-Host "   THAT BAI"; $fail = $true }
}

# Ban kiem thu co kiem tra tran 64-bit (luon build, ke ca o che do release).
Write-Host ""
Write-Host ">> tests\unit_tests.cpp  ->  bin\unit_tests_checked.exe (ST_CHECK_OVERFLOW)"
& $gpp ($common + @("-O1", "-g", "-DST_CHECK_OVERFLOW")) "tests\unit_tests.cpp" -o "bin\unit_tests_checked.exe"
if ($LASTEXITCODE -ne 0) { Write-Host "   THAT BAI"; $fail = $true }

Write-Host ""
if ($fail) { Write-Host "BIEN DICH THAT BAI." -ForegroundColor Red; exit 1 }
Write-Host "BIEN DICH THANH CONG. Cac tep thuc thi nam trong .\bin\" -ForegroundColor Green
Get-ChildItem bin -Filter *.exe | ForEach-Object { Write-Host ("  " + $_.Name + "  (" + [math]::Round($_.Length/1KB) + " KB)") }
exit 0
