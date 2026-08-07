# Ket qua thuc nghiem - Chuyen de 6: Segment Tree + Lazy Propagation

Sinh tu dong boi `run_experiments.ps1` luc 2026-08-07 17:35:38 +07:00.
Thong tin moi truong chay: xem [environment.md](environment.md).

Ky hieu: **N** = so phan tu, **Q** = so thao tac, **ns/op** = nano giay
trung binh cho mot thao tac. Moi o la gia tri NHO NHAT qua 5 lan chay.

## Bang 1. Thoi gian theo kich thuoc N (tai hon hop 50% cap nhat / 50% truy van)

| N | Q | Segment Tree (ms) | ns/op | Mang thuong (ms) | ns/op | Ty le tang toc |
|---:|---:|---:|---:|---:|---:|---:|
| 1000 | 200000 | 65.794 | 329.0 | 39.590 | 197.9 | 0.6x |
| 10000 | 200000 | 90.231 | 451.2 | 321.012 | 1605.1 | 3.6x |
| 100000 | 200000 | 201.107 | 1005.5 | 3167.593 | 15838.0 | 15.8x |
| 1000000 | 200000 | 323.018 | 1615.1 | khong do (qua cham) | - | - |

## Bang 2. Thoi gian cua Segment Tree theo KIEU TAI

| N | mixed (ns/op) | update (ns/op) | query (ns/op) | point (ns/op) | full (ns/op) |
|---:|---:|---:|---:|---:|---:|
| 1000 | 329.0 | 391.8 | 218.6 | 157.9 | 9.0 |
| 10000 | 451.2 | 590.4 | 302.4 | 206.8 | 8.8 |
| 100000 | 1005.5 | 1009.7 | 531.3 | 371.0 | 8.8 |
| 1000000 | 1615.1 | 1875.6 | 1024.6 | 694.8 | 9.2 |

Kieu tai `full` (moi thao tac phu toan doan [1, N]) la truong hop lazy co loi
nhat: cap nhat dung ngay o nut goc nen chi ton O(1).

## Bang 3. Thoi gian dung cay va bo nho su dung

| N | Thoi gian build (ms) | Bo nho cay (MB) | Byte / phan tu |
|---:|---:|---:|---:|
| 1000 | 0.102 | 0.06 | 64.0 |
| 10000 | 0.132 | 0.61 | 64.0 |
| 100000 | 4.106 | 6.10 | 64.0 |
| 1000000 | 37.314 | 61.04 | 64.0 |

Cay cap phat 4N o cho gia tri gop va 4N o cho the lazy, moi o 8 byte,
nen ly thuyet la 64 byte cho moi phan tu cua mang goc.

## Bang 4. Thoi gian tron goi tren tep du lieu that

`Tong` bao gom khoi dong tien trinh, doc tep, xu ly va ghi tep.
`Build` la thoi gian dung cay, `Thao tac` la thoi gian thuc hien Q thao tac.

| Tep du lieu | N | Q | Cai dat | Tong (ms) | Build (ms) | Thao tac (ms) |
|---|---:|---:|---|---:|---:|---:|
| small_n1000_q1000.txt | 1000 | 1000 | segtree | 33.459 | 0.055 | 0.362 |
| small_n1000_q1000.txt | 1000 | 1000 | naive | 28.169 | 0.001 | 0.326 |
| medium_n50000_q50000.txt | 50000 | 50000 | segtree | 81.851 | 1.634 | 39.512 |
| medium_n50000_q50000.txt | 50000 | 50000 | naive | 466.952 | 0.034 | 424.693 |
| large_n1000000_q1000000.txt | 1000000 | 1000000 | segtree | 2034.136 | 29.470 | 1786.930 |
| adv_full_n200000_q200000.txt | 200000 | 200000 | segtree | 81.506 | 6.250 | 14.077 |
| adv_point_n200000_q200000.txt | 200000 | 200000 | segtree | 216.261 | 5.212 | 143.021 |
| adv_prefix_n200000_q200000.txt | 200000 | 200000 | segtree | 211.414 | 5.732 | 137.571 |

## Nhan xet

1. Thoi gian moi thao tac cua Segment Tree tang **tuyen tinh theo log N**:
   N tang 10 lan (log2 N tang khoang 3.32) thi ns/op chi tang mot luong nho,
   hoan toan khac voi mang thuong tang gan dung 10 lan.
2. Phan tang ns/op o N lon con den tu **hieu ung bo nho dem**: cay 1 trieu
   phan tu chiem khoang 64 MB, vuot xa bo nho dem cap ba cua CPU.
3. O kieu tai `full`, cap nhat dung ngay tai nut goc nen thoi gian gan nhu
   khong phu thuoc N - day chinh la loi ich cua lazy propagation.
4. Ty le tang toc so voi mang thuong lon dan theo N, dung nhu du doan cua
   phan tich do phuc tap O(N) so voi O(log N).

