# Khung slide — Chuyên đề 6: Segment Tree và Lazy Propagation

**Mục tiêu: 12–18 trang, demo 10–15 phút.** Khung dưới đây có 16 trang, khớp yêu
cầu của đề. Mỗi trang ghi rõ *nội dung*, *hình/số liệu cần đưa lên*, và *lời
thoại gợi ý*.

> **[NHÓM TỰ VIẾT]** Chuyển khung này sang PowerPoint/Google Slides. Mọi số liệu
> đã có sẵn trong `results/results.md`; ba hình đã có sẵn trong
> `report/figures/*.svg` (chèn thẳng vào slide được).

---

## Trang 1 — Bìa

* Tên chuyên đề, Nhóm 3, danh sách thành viên, giảng viên, ngày báo cáo.

## Trang 2 — Bài toán

* Mảng `a[1..N]`, hai thao tác:
  * `1 L R V` — cộng `V` vào mọi phần tử trong `[L, R]`
  * `2 L R` — trả về tổng các phần tử trong `[L, R]`
* Ràng buộc: `N, Q ≤ 10^6`, `|a_i|, |V| ≤ 10^9`.
* **Lời thoại:** nhấn mạnh *cả hai* thao tác đều tác động lên **một đoạn**, đó
  là điều khiến bài toán khó.

## Trang 3 — Vì sao cách làm đơn giản không đủ

| Cách làm | Cập nhật đoạn | Truy vấn đoạn |
|---|---|---|
| Mảng thường | `O(N)` | `O(N)` |
| Tổng tiền tố | `O(N)` | `O(1)` |
| **Segment Tree + lazy** | **`O(log N)`** | **`O(log N)`** |

* **Lời thoại:** với `Q = 10^6` thao tác trên `N = 10^6`, cách `O(N)` cần khoảng
  `10^12` phép tính — không khả thi.

## Trang 4 — Ý tưởng: mỗi nút đại diện một đoạn

* Hình cây cho `N = 8`: gốc giữ `[0,7]`, hai con giữ `[0,3]` và `[4,7]`, …
* Nhấn mạnh: đây là **phân hoạch phân cấp**, không phải "cây nhị phân hoàn
  chỉnh trong mọi trường hợp".
* **[NHÓM TỰ VIẾT]** Vẽ hình cây (có thể vẽ bằng PowerPoint hoặc draw.io).

## Trang 5 — Truy vấn đoạn chạm bao nhiêu nút?

* Hình: tô màu đoạn `[2, 6]` trên cây `N = 8`; chỉ ra các nút "phủ toàn bộ".
* Kết luận: mỗi mức có nhiều nhất 2 nút "giao một phần" → `O(log N)` nút.

## Trang 6 — Vấn đề: cập nhật đoạn còn tốn `O(N)`

* Cộng `V` vào `[1, N]` mà đi xuống tận lá thì phải sửa `N` lá.
* Câu hỏi dẫn dắt: *có nhất thiết phải sửa ngay không?*

## Trang 7 — Lazy propagation: "ghi nợ" thay vì trả ngay

* Khi một nút **nằm trọn** trong đoạn cập nhật:
  * cập nhật giá trị gộp của nó **ngay**;
  * ghi phép cập nhật vào `lazy[v]` và **dừng lại**.
* Chỉ "trả nợ" khi thật sự cần đi xuống con.

## Trang 8 — Hai bất biến (trang quan trọng nhất)

* **(I1)** `tree[v]` **đã đúng**, kể cả phần `lazy[v]`.
* **(I2)** `lazy[v]` **chưa** được áp cho hai con.
* Hệ quả: đọc `tree[v]` luôn hợp lệ; chỉ khi **đi xuống con** mới cần `push_down`.
* **Lời thoại:** nếu chỉ nhớ một trang trong bài này thì nhớ trang này.

## Trang 9 — Ba trường hợp của đệ quy

| Quan hệ `[tl,tr]` với `[l,r]` | Hành động |
|---|---|
| Nằm ngoài | dừng ngay |
| Phủ toàn bộ | `apply` rồi **dừng** ← chỗ lazy tiết kiệm |
| Giao một phần | `push_down`, đệ quy hai con, rồi `pull` |

## Trang 10 — Ba thao tác nguyên thủy (mã)

```cpp
void apply(int v, int len, Tag t) {
    tree_[v] += t * len;   // cộng t vào len phần tử → tổng tăng t*len
    lazy_[v] += t;         // hợp thành thẻ
}
void push_down(int v, int tl, int tm, int tr) {
    if (lazy_[v] == 0) return;
    apply(2*v,   tm - tl + 1, lazy_[v]);
    apply(2*v+1, tr - tm,     lazy_[v]);
    lazy_[v] = 0;
}
void pull(int v) { tree_[v] = tree_[2*v] + tree_[2*v+1]; }
```

* **Lời thoại:** nhấn mạnh chữ `* len` — đây là chỗ phân biệt *giá trị của cây*
  (tổng cả đoạn) với *giá trị của cập nhật* (lượng cộng cho một phần tử).

## Trang 11 — Cái bẫy: một lỗi có thật của nhóm

```cpp
build:  t[v] = t[2v] + t[2v+1];    // gộp bằng TỔNG
query:  return min(s1, s2);        // gộp bằng MIN  ← không nhất quán!
```

* Với `a = [1, 2, 8, 9, 3]`, truy vấn min trên `[3, 4]` trả `17` thay vì `8`.
* Vì sao khó thấy: chỉ lộ khi đoạn truy vấn phủ trọn một **nút trong**.
* Bài học: **phép gộp phải nằm ở một chỗ duy nhất trong mã.**

## Trang 12 — Kiểm thử: bốn tầng bảo vệ

| Tầng | Kết quả |
|---|---|
| Kiểm thử đơn vị (5 nhóm) | 71 đạt / 0 hỏng |
| Bản có kiểm tra tràn 64-bit | 71 đạt / 0 hỏng |
| 12 test cố định | 12 đạt / 0 hỏng |
| Stress test 3000 vòng (~1,5 triệu thao tác) | không sai lệch |

* Nhấn mạnh: đáp án chuẩn được xác thực bằng **hai cài đặt độc lập** cộng với
  **tính tay**.

## Trang 13 — Thực nghiệm: kiểm chứng `O(log N)`

* Chèn **`report/figures/fig2_logn.svg`**.
* Hồi quy: `ns/op ≈ 132,8 · log₂ N − 1135,5`, `R² = 0,936`.
* **Lời thoại:** điểm nằm gần một đường thẳng khi vẽ theo `log₂ N` → bằng chứng
  thực nghiệm cho `O(log N)`.

## Trang 14 — Thực nghiệm: so với mảng thường

* Chèn **`report/figures/fig1_scaling.svg`**.

| `N` | Tăng tốc so với mảng thường |
|---:|---:|
| 1 000 | **0,6×** (mảng thường còn nhanh hơn!) |
| 10 000 | 3,6× |
| 100 000 | **15,8×** |

* **Lời thoại:** phải nói thẳng ở `N` nhỏ mảng thường thắng — **không phóng đại
  khả năng thuật toán**.

## Trang 15 — Số liệu thuyết phục nhất: kiểu tải `full`

* Chèn **`report/figures/fig3_workloads.svg`**.

| `N` | ns/op ở kiểu tải `full` |
|---:|---:|
| 1 000 | 9,0 |
| 10 000 | 8,8 |
| 100 000 | 8,8 |
| 1 000 000 | 9,2 |

* **Lời thoại:** `N` tăng **1000 lần**, thời gian **không đổi** — vì mọi cập
  nhật dừng ngay ở nút gốc. Đó chính xác là lợi ích của lazy propagation.

## Trang 16 — Mở rộng, ứng dụng và kết luận

* **Mở rộng đã cài:** gán đoạn + min/max (thẻ không giao hoán); affine
  `x ↦ a·x + b (mod p)`.
* **Ứng dụng:** duy trì tổng trên chuỗi thời gian có cập nhật theo khoảng —
  ví dụ hệ thống đặt phòng.
* **Giới hạn:** cần phép gộp có tính kết hợp; thẻ phải hợp thành được với kích
  thước hằng số; chỉ có lợi khi `N` đủ lớn.

---

## Kịch bản demo 10–15 phút

| Phút | Nội dung | Lệnh |
|---|---|---|
| 0–2 | Giới thiệu cấu trúc dự án, mở `src/segment_tree.hpp`, chỉ ba hàm `apply`/`push_down`/`pull` | — |
| 2–4 | Biên dịch trực tiếp trên máy | `.\build.ps1` |
| 4–8 | Chạy toàn bộ kiểm thử, chỉ ra 71 + 12 test và stress test | `.\run_tests.ps1` |
| 8–10 | Chạy ví dụ nhỏ, đối chiếu với kết quả tính tay trên slide | `.\bin\segtree.exe tests\cases\01_basic_sample.in` |
| 10–13 | Chạy dữ liệu `N = Q = 10^6`, cho thấy khoảng 2 giây; chạy lại bằng `--engine=naive` trên dữ liệu vừa để so | `.\bin\segtree.exe --time data\large_n1000000_q1000000.txt out.txt` |
| 13–15 | Mở `results/results.md`, đi qua bốn bảng, kết luận | — |

**Mẹo demo:** chạy `.\build.ps1` và `.\run_experiments.ps1` **trước** buổi báo
cáo để có sẵn `bin/` và `data/`; trong lúc demo chỉ chạy lại phần nhanh.
