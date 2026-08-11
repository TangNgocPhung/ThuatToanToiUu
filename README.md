# Chuyên đề 6 — Cây phân đoạn (Segment Tree) và Lazy Propagation

Cài đặt, kiểm thử và thực nghiệm cây phân đoạn có lan truyền lười (lazy
propagation) cho bài toán **cập nhật đoạn + truy vấn tổng đoạn**, kèm hai biến
thể mở rộng (gán đoạn với min/max, và cập nhật affine theo modulo).

---

## 1. Bài toán

Cho mảng `a[1..N]` các số nguyên. Cần hỗ trợ `Q` thao tác, mỗi thao tác thuộc
một trong hai loại:

| Thao tác | Ý nghĩa | Độ phức tạp |
|---|---|---|
| `1 L R V` | Cộng `V` vào **mọi** phần tử `a[L], a[L+1], …, a[R]` | `O(log N)` |
| `2 L R`   | Trả về tổng `a[L] + a[L+1] + … + a[R]`                | `O(log N)` |

Dựng cây tốn `O(N)` thời gian và `O(N)` bộ nhớ.

### Ràng buộc và giả thiết

| Đại lượng | Ràng buộc |
|---|---|
| `N` | `1 ≤ N ≤ 10^6` |
| `Q` | `0 ≤ Q ≤ 10^6` |
| `a_i` | số nguyên, `\|a_i\| ≤ 10^9` |
| `V` | số nguyên, `\|V\| ≤ 10^9` |
| `L`, `R` | `1 ≤ L ≤ R ≤ N` (chỉ số tính từ 1) |

**Điều kiện an toàn 64-bit.** Đặt `A = max|a_i|` và `T = Σ|V_j|` (tổng trị tuyệt
đối của mọi lượng cập nhật). Khi đó mỗi phần tử có trị tuyệt đối không vượt quá
`A + T`, và tổng một đoạn không vượt quá `N · (A + T)`. Chương trình dùng
`long long` (64-bit) nên **cần** `N · (A + T) < 2^63 ≈ 9.22 · 10^18`. Bản dịch
với cờ `-DST_CHECK_OVERFLOW` sẽ kiểm tra điều kiện này khi chạy và dừng chương
trình nếu bị vi phạm (xem mục 6).

---

## 2. Cấu trúc thư mục

```
.
├── README.md                 tệp này
├── Makefile                  biên dịch toàn bộ (Linux / macOS / MSYS2)
│
├── scripts/                  script điều khiển (Windows / PowerShell)
│   ├── build.ps1              biên dịch toàn bộ
│   ├── run_tests.ps1          chạy toàn bộ bộ kiểm thử
│   ├── run_experiments.ps1    chạy toàn bộ thực nghiệm đo thời gian
│   └── make_figures.ps1       sinh hình minh họa (SVG) từ số liệu benchmark
│
├── src/                      mã nguồn chính
│   ├── segment_tree.hpp      LỚP CHÍNH: cập nhật đoạn (cộng) + truy vấn tổng
│   ├── segment_tree_ext.hpp  mở rộng: gán đoạn + min/max, và affine mod p
│   ├── naive.hpp             lời giải mảng thường O(N), dùng làm chuẩn đối chiếu
│   ├── main.cpp              chương trình giải bài toán kiểm chứng
│   └── bench.cpp             chương trình đo thời gian
│
├── tests/                    kiểm thử
│   ├── unit_tests.cpp        test cơ bản / biên / đối kháng / ngẫu nhiên / mở rộng
│   ├── stress_test.cpp       stress test ngẫu nhiên đối chiếu mảng thường
│   ├── generator.cpp         bộ sinh dữ liệu vào
│   ├── make_expected.ps1     sinh đáp án chuẩn cho test cố định
│   └── cases/                12 test cố định (.in) kèm đáp án chuẩn (.out)
│
├── data/                     dữ liệu vào dùng cho thực nghiệm (sinh tự động)
├── results/                  bảng kết quả, nhật ký chạy, thông tin môi trường
├── report/                   báo cáo kỹ thuật và hình minh họa
└── slides/                   tệp trình bày
```

---

## 3. Yêu cầu môi trường

* Trình biên dịch hỗ trợ **C++17**: `g++ ≥ 7`, `clang++ ≥ 6`, hoặc MSVC 2019+.
* Windows: khuyến nghị **MSYS2 / MinGW-w64** (`pacman -S mingw-w64-ucrt-x86_64-gcc`).
* PowerShell 5.1 trở lên (có sẵn trên Windows 10/11) để chạy các script `.ps1`.

Mã nguồn không dùng thư viện ngoài nào. Hai tệp `segment_tree.hpp` và
`naive.hpp` là header-only, có thể chép sang dự án khác mà dùng ngay.

---

## 4. Biên dịch

### Windows (PowerShell)

```powershell
.\scripts\build.ps1
```

Script tự dò tìm `g++` trong `PATH`, sau đó thử các vị trí cài đặt thông dụng
(`C:\msys64\ucrt64\bin`, `C:\msys64\mingw64\bin`, `C:\mingw64\bin`, …). Nếu
trình biên dịch nằm ở chỗ khác:

```powershell
.\scripts\build.ps1 -Compiler "D:\tools\mingw64\bin\g++.exe"
```

Bản gỡ lỗi (bật `assert` và kiểm tra tràn 64-bit):

```powershell
.\scripts\build.ps1 -Debug
```

### Linux / macOS / MSYS2 shell

```bash
make
```

### Sản phẩm biên dịch

Mọi tệp thực thi được đặt trong `bin/`:

| Tệp | Vai trò |
|---|---|
| `segtree` | giải bài toán kiểm chứng |
| `bench` | đo thời gian trong bộ nhớ |
| `generator` | sinh dữ liệu vào |
| `stress_test` | stress test ngẫu nhiên đối chiếu |
| `unit_tests` | bộ kiểm thử đơn vị |
| `unit_tests_checked` | như trên, thêm kiểm tra tràn 64-bit |

> **Lưu ý riêng cho Windows.** `build.ps1` liên kết tĩnh
> (`-static -static-libgcc -static-libstdc++`) một cách có chủ đích. Nếu liên
> kết động, chương trình có thể nạp nhầm `libstdc++-6.dll` của một bộ công cụ
> khác đang nằm trong `PATH` (Git for Windows đặt một bản tại
> `C:\Program Files\Git\mingw64\bin`) và **đổ vỡ với lỗi 0xC0000005 ngay khi
> tạo một đối tượng `std::ofstream`**. Liên kết tĩnh khiến các tệp `.exe` độc
> lập hoàn toàn và chạy được trên máy chưa cài MSYS2.

---

## 5. Định dạng dữ liệu vào–ra

### Dữ liệu vào

```
N Q
a_1 a_2 … a_N
<Q dòng thao tác>
```

Mỗi dòng thao tác có một trong hai dạng:

```
1 L R V     cộng V vào mọi phần tử trong [L, R]
2 L R       in ra tổng các phần tử trong [L, R]
```

Chỉ số **tính từ 1**. Các số cách nhau bởi khoảng trắng hoặc xuống dòng tùy ý.

### Dữ liệu ra

Mỗi thao tác loại `2` in ra **một dòng** chứa một số nguyên 64-bit.

### Ví dụ

`tests/cases/01_basic_sample.in`

```
5 6
1 2 8 9 3
2 1 5
2 2 4
1 2 4 5
2 1 5
2 2 4
2 3 3
```

Kết quả (`tests/cases/01_basic_sample.out`)

```
23
19
38
34
13
```

Giải thích: ban đầu tổng cả mảng là `1+2+8+9+3 = 23`, tổng `a[2..4] = 19`. Sau
khi cộng `5` vào `[2,4]` mảng thành `1 7 13 14 3`, nên tổng cả mảng là `38`,
`a[2..4] = 34` và `a[3] = 13`.

---

## 6. Chạy chương trình

```powershell
# đọc bàn phím, in ra màn hình
.\bin\segtree.exe

# đọc tệp, ghi tệp
.\bin\segtree.exe data\small_n1000_q1000.txt out.txt

# chạy lời giải mảng thường để đối chiếu
.\bin\segtree.exe --engine=naive data\small_n1000_q1000.txt out_naive.txt

# in thời gian xử lý ra luồng lỗi chuẩn
.\bin\segtree.exe --time data\small_n1000_q1000.txt out.txt

# ghi số liệu đo ra tệp CSV (đáng tin cậy hơn --time khi chạy trong script)
.\bin\segtree.exe --stats=stats.csv data\small_n1000_q1000.txt out.txt
```

### Bảng tham số

**`bin/segtree`**

| Tham số | Mặc định | Ý nghĩa |
|---|---|---|
| `[tệp_vào]` | `stdin` | tệp dữ liệu vào |
| `[tệp_ra]` | `stdout` | tệp kết quả |
| `--engine=segtree\|naive` | `segtree` | chọn cài đặt |
| `--time` | tắt | in thời gian ra `stderr` |
| `--stats=<tệp>` | — | ghi `engine,n,q,build_ms,ops_ms,total_ms` ra tệp CSV |
| `-h`, `--help` | — | trợ giúp |

Mã thoát: `0` thành công, `2` dữ liệu vào sai định dạng hoặc tham số không hợp lệ.

**`bin/generator`**

| Tham số | Mặc định | Ý nghĩa |
|---|---|---|
| `--n=<int>` | `1000` | số phần tử `N` |
| `--q=<int>` | `1000` | số thao tác `Q` |
| `--seed=<u64>` | `1` | hạt giống ngẫu nhiên (đảm bảo tái lập) |
| `--maxval=<ll>` | `10^9` | `\|a_i\| ≤ maxval` |
| `--maxadd=<ll>` | `10^9` | `\|V\| ≤ maxadd` |
| `--pupd=<0..1>` | `0.5` | tỉ lệ thao tác cập nhật |
| `--mode=<mode>` | `mixed` | `mixed`, `full`, `point`, `small`, `prefix`, `suffix` |
| `--out=<tệp>` | `stdout` | tệp kết quả |

**`bin/bench`**

| Tham số | Mặc định | Ý nghĩa |
|---|---|---|
| `--sizes=a,b,c` | `1000,10000,100000,1000000` | các giá trị `N` cần đo |
| `--ops=<int>` | `200000` | số thao tác mỗi cấu hình |
| `--repeat=<int>` | `3` | số lần lặp, lấy giá trị nhỏ nhất |
| `--with-naive` | tắt | đo thêm lời giải mảng thường |
| `--naive-max-work=<x>` | `2.5e10` | ngưỡng `N·Q` để còn chạy mảng thường |
| `--out=<tệp>` | `stdout` | ghi CSV ra tệp |

**`bin/stress_test`**

| Tham số | Mặc định | Ý nghĩa |
|---|---|---|
| `--rounds=<int>` | `2000` | số kịch bản ngẫu nhiên |
| `--seed=<u64>` | `1` | hạt giống |
| `--maxn=<int>` | `200` | `N` tối đa |
| `--ops=<int>` | `500` | số thao tác mỗi kịch bản |
| `--maxval=<ll>` | `10^9` | biên độ giá trị |
| `--quiet` | tắt | không in tiến độ |

Khi phát hiện sai lệch, chương trình ghi kịch bản lỗi ra `stress_fail.in` và
thoát với mã `1`.

---

## 7. Chạy kiểm thử

```powershell
.\scripts\run_tests.ps1                       # bộ đầy đủ
.\scripts\run_tests.ps1 -StressRounds 20000   # stress test lâu hơn
.\scripts\run_tests.ps1 -SkipStress           # bỏ qua stress test
```

Bộ kiểm thử gồm bốn bước:

1. **Kiểm thử đơn vị** — 71 khẳng định chia thành năm nhóm: cơ bản, biên, đối
   kháng, ngẫu nhiên đối chiếu, và mở rộng.
2. **Kiểm thử đơn vị bản `ST_CHECK_OVERFLOW`** — chạy lại toàn bộ với `assert`
   và kiểm tra tràn 64-bit bật.
3. **12 test cố định** trong `tests/cases`, so từng dòng với đáp án chuẩn.
4. **Stress test** đối chiếu với lời giải mảng thường.

Nhật ký đầy đủ được ghi vào `results/test_log.txt`. Mã thoát `0` nghĩa là mọi
bước đều đạt.

**Đáp án chuẩn được xác thực chéo.** `tests/make_expected.ps1` chạy mỗi tệp
`.in` bằng **cả hai** cài đặt (cây phân đoạn và mảng thường) và chỉ ghi ra
`.out` khi hai kết quả trùng nhau. Nhờ vậy đáp án chuẩn không bao giờ được
"chứng thực" bởi chính cài đặt đang cần kiểm tra. Ngoài ra, tám test đầu tiên
có kết quả nhỏ, đã được **tính tay** và đối chiếu độc lập.

---

## 8. Chạy thực nghiệm

```powershell
.\scripts\run_experiments.ps1           # bộ đầy đủ (vài phút)
.\scripts\run_experiments.ps1 -Quick    # bản rút gọn
```

Script sinh dữ liệu vào `data/`, đo thời gian, rồi ghi ra:

| Tệp | Nội dung |
|---|---|
| `results/environment.md` | cấu hình máy, phiên bản trình biên dịch, cờ dịch, đơn vị đo |
| `results/benchmark.csv` | số liệu thô đo trong bộ nhớ |
| `results/end_to_end.csv` | số liệu thô đo trọn gói trên tệp dữ liệu thật |
| `results/results.md` | bốn bảng kết quả định dạng Markdown, sẵn để đưa vào báo cáo |

---

## 9. Tóm tắt thiết kế lớp `SegmentTree`

```cpp
class SegmentTree {
public:
    using Value = long long;   // GIÁ TRỊ TỔNG HỢP tại nút: tổng của cả đoạn
    using Tag   = long long;   // GIÁ TRỊ CẬP NHẬT hoãn: lượng cộng cho MỘT phần tử

    void      build(const std::vector<Value>& a);   // O(N)
    void      update_range(int l, int r, Tag v);    // O(log N)
    Value     query_range(int l, int r);            // O(log N)
private:
    void  apply(int v, int len, Tag t);             // áp thẻ lên một nút
    void  push_down(int v, int tl, int tm, int tr); // đẩy thẻ xuống hai con
    void  pull(int v);                              // tính lại cha từ hai con
};
```

`Value` và `Tag` được **đặt tên tách bạch** theo yêu cầu của đề. Tuy cùng biểu
diễn bằng `long long`, chúng khác nhau về ngữ nghĩa: `Value` là *tổng của cả
đoạn*, còn `Tag` là *lượng cộng cho từng phần tử*. Chính vì vậy `apply` phải
nhân thẻ với độ dài đoạn: `tree[v] += t * len`. Nhầm lẫn hai khái niệm này là
nguồn lỗi phổ biến nhất khi cài lazy propagation.

### Hai bất biến của cây

* **(I1)** `tree[v]` là giá trị gộp **đúng** của đoạn mà nút `v` quản lý, đã bao
  gồm mọi cập nhật đã tác động lên `v`, **kể cả** `lazy[v]`.
* **(I2)** `lazy[v]` là cập nhật đã được áp cho `tree[v]` nhưng **chưa** được áp
  cho hai cây con.

Hệ quả: đọc `tree[v]` luôn hợp lệ; chỉ khi cần **đi xuống con** mới phải gọi
`push_down` để khôi phục (I1) cho các con.

### Ba trường hợp của đệ quy

| Quan hệ giữa đoạn của nút `[tl,tr]` và đoạn truy vấn `[l,r]` | Hành động |
|---|---|
| Nằm ngoài nhau (`r < tl` hoặc `tr < l`) | dừng ngay, trả phần tử trung hòa |
| `[tl,tr]` nằm **trọn** trong `[l,r]` | `apply` rồi **dừng** — đây là chỗ lazy tiết kiệm |
| Giao nhau một phần | `push_down` rồi đệ quy xuống hai con, sau đó `pull` |

### Vì sao cấp phát `4N` ô nhớ

Cây phân đoạn đệ quy trên `[0, N-1]` **không** phải cây nhị phân hoàn chỉnh khi
`N` không là lũy thừa của 2. Chiều cao là `h = ⌈log₂ N⌉`, và chỉ số nút lớn
nhất có thể chạm tới nhỏ hơn `2^(h+1) ≤ 4N`. Vì vậy `4N` là cận trên an toàn phổ
dụng cho mọi `N ≥ 1`. Điều thật sự quan trọng là **phân hoạch đoạn** và **chiều
cao logarit**, chứ không phải tính "hoàn chỉnh" của cây.

---

## 10. Mở rộng đã cài đặt

Nằm trong `src/segment_tree_ext.hpp`; cả hai đều được stress test đối chiếu với
mảng thường trong `tests/unit_tests.cpp`.

### `SegmentTreeAssignAdd` — gán đoạn + cộng đoạn, truy vấn tổng/min/max

* `Value` là bộ ba `{sum, min, max}`.
* `Tag` là bộ ba `{has_assign, assign_val, add_val}` với ngữ nghĩa "nếu có gán
  thì **gán trước**, sau đó cộng".
* Hợp thành thẻ **không giao hoán**: một phép gán mới **xóa sạch** mọi phép cộng
  đứng trước nó. Mọi dãy thao tác gán/cộng đều rút gọn được về đúng dạng này,
  nên thẻ vẫn có kích thước hằng số.

### `SegmentTreeAffine` — cập nhật affine `x ↦ a·x + b (mod p)`

* `Tag` là cặp `(a, b)`, thẻ đơn vị là `(1, 0)`.
* Hợp thành: áp `(a₁,b₁)` trước rồi `(a₂,b₂)` sau cho
  `x ↦ (a₂a₁)·x + (a₂b₁ + b₂)` — **thứ tự quan trọng**, đây là ví dụ chuẩn mực
  cho hợp thành thẻ không giao hoán.
* Gán đoạn là trường hợp riêng `affine(0, x)`; cộng đoạn là `affine(1, x)`.

---

## 11. Khắc phục sự cố

| Hiện tượng | Nguyên nhân và cách xử lý |
|---|---|
| `build.ps1` báo "Khong tim thay g++" | Cài MSYS2/MinGW-w64, hoặc truyền `-Compiler <đường dẫn g++.exe>`. |
| Biên dịch thất bại, không có thông báo lỗi nào | Thư mục `bin` của trình biên dịch chưa nằm trong `PATH` nên `cc1plus.exe` không nạp được DLL. `build.ps1` đã tự xử lý; nếu gọi `g++` thủ công, hãy thêm thư mục đó vào `PATH`. |
| Chương trình đổ vỡ với `0xC0000005` | Xung đột DLL `libstdc++-6.dll`. Hãy biên dịch có `-static` (mặc định của `build.ps1`). |
| `Loi dinh dang du lieu vao: thieu loai truy van` | Số dòng thao tác trong tệp ít hơn giá trị `Q` đã khai ở dòng đầu. |
| `assert` thất bại kèm chữ `TRAN 64-bit` | Dữ liệu vi phạm điều kiện an toàn 64-bit ở mục 1. Hãy giảm `N`, `|a_i|` hoặc `|V|`. |
| Không chạy được `.ps1` | `Set-ExecutionPolicy -Scope Process -ExecutionPolicy Bypass` rồi chạy lại. |

---

## 12. Ứng dụng minh họa

Bài toán "cộng vào đoạn, hỏi tổng đoạn" mô hình hóa trực tiếp việc **duy trì
tổng trên chuỗi dữ liệu thời gian có cập nhật theo khoảng và truy vấn tức thì**.

Ví dụ cụ thể: một hệ thống đặt phòng ghi số khách theo từng ngày trong năm
(`N = 365`). Một đơn đặt phòng dài ngày là thao tác `1 L R V` (cộng `V` khách
vào mọi ngày trong khoảng), còn câu hỏi "tổng số đêm khách trong quý II là bao
nhiêu" là thao tác `2 L R`. Nếu thay `combine` bằng `max`, cùng cấu trúc đó trả
lời được "ngày đông khách nhất trong khoảng là bao nhiêu khách" — hữu ích để
kiểm tra ràng buộc sức chứa.

Cần nói rõ **giới hạn**: cấu trúc này chỉ phù hợp khi phép gộp có **tính kết
hợp** và phép cập nhật **hợp thành được thành thẻ kích thước hằng số**. Nó
không thay thế được cơ sở dữ liệu, không xử lý được truy vấn tùy ý trên nhiều
chiều, và với dữ liệu chỉ đọc thì mảng tổng tiền tố đơn giản hơn và nhanh hơn.

---

## 13. Tài liệu tham khảo

1. Cormen, T. H., Leiserson, C. E., Rivest, R. L., Stein, C. *Introduction to
   Algorithms*, 4th ed. MIT Press, 2022 — chương về cấu trúc dữ liệu bổ trợ.
2. Halim, S., Halim, F., Effendy, S. *Competitive Programming 4*, Book 2.
   Lulu, 2020 — mục về Segment Tree và Lazy Propagation.
3. cp-algorithms.com, *Segment Tree*. https://cp-algorithms.com/data_structures/segment_tree.html
4. Laaksonen, A. *Competitive Programmer's Handbook*, 2018 — chương 28,
   "Segment trees revisited".
