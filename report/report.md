# Báo cáo kỹ thuật — Chuyên đề 6
# Cây phân đoạn (Segment Tree) và Lazy Propagation

> **Trạng thái tài liệu.** Đây là **khung báo cáo** đã được điền sẵn toàn bộ nội
> dung kỹ thuật rút ra trực tiếp từ mã nguồn và số liệu thực nghiệm trong
> `results/`. Những chỗ đánh dấu **[NHÓM TỰ VIẾT]** là phần nhóm phải tự bổ
> sung: thông tin thành viên, phân công, diễn giải mở rộng, và các nhận xét
> riêng. Mục tiêu độ dài 20–30 trang (không tính phụ lục mã nguồn và dữ liệu).

---

## Trang bìa **[NHÓM TỰ VIẾT]**

* Tên trường, khoa, tên học phần
* Tên chuyên đề: *Cây phân đoạn (Segment Tree) và Lazy Propagation*
* Nhóm 3 — Cấu trúc dữ liệu nâng cao
* Danh sách thành viên và mã học viên
* Giảng viên hướng dẫn
* Thời gian thực hiện

## Bảng phân công **[NHÓM TỰ VIẾT]**

| Thành viên | Phần phụ trách | Phần lõi thuật toán có thể giải thích |
|---|---|---|
| | lý thuyết, mục 2–3 | bất biến cây, ba trường hợp đệ quy |
| | cài đặt, mục 4 | `apply`, `push_down`, `pull` |
| | kiểm thử, mục 5 | thiết kế test biên và test đối kháng |
| | thực nghiệm, mục 6 | phương pháp đo, phân tích số liệu |

> Yêu cầu của đề: **mọi thành viên phải giải thích được phần được phân công
> *và* phần lõi của thuật toán.**

---

## Mục 1. Phát biểu bài toán

### 1.1. Đầu vào

Mảng số nguyên `a[1..N]` và một dãy `Q` thao tác.

### 1.2. Hai thao tác cần hỗ trợ

| Thao tác | Ý nghĩa |
|---|---|
| `1 L R V` | cộng `V` vào mọi phần tử `a[i]` với `L ≤ i ≤ R` |
| `2 L R` | trả về `Σ a[i]` với `L ≤ i ≤ R` |

### 1.3. Đầu ra

Với mỗi thao tác loại `2`, in ra một dòng chứa một số nguyên 64-bit.

### 1.4. Ràng buộc

| Đại lượng | Ràng buộc |
|---|---|
| `N` | `1 ≤ N ≤ 10^6` |
| `Q` | `0 ≤ Q ≤ 10^6` |
| `a_i` | `|a_i| ≤ 10^9` |
| `V` | `|V| ≤ 10^9` |
| `L, R` | `1 ≤ L ≤ R ≤ N`, chỉ số tính từ 1 |

### 1.5. Giả thiết và điều kiện an toàn số học

Đặt `A = max|a_i|` và `T = Σ|V_j|` lấy trên mọi thao tác cập nhật. Khi đó:

* trị tuyệt đối của mỗi phần tử tại mọi thời điểm không vượt quá `A + T`;
* trị tuyệt đối của tổng một đoạn không vượt quá `N · (A + T)`.

Vì cài đặt dùng `long long` (64-bit có dấu), **điều kiện an toàn** là

```
N · (A + T) < 2^63 − 1 ≈ 9.22 · 10^18
```

Đây là một giả thiết **bắt buộc phải nêu rõ**, vì với `N = 10^6`,
`A = 10^9` và `Q = 10^6` thao tác mỗi lần cộng `10^9`, cận trên trở thành
`10^6 · (10^9 + 10^6 · 10^9) = 10^21`, **vượt quá** khả năng biểu diễn của
`long long`. Bản dịch với cờ `-DST_CHECK_OVERFLOW` kiểm tra điều kiện này ngay
khi chạy (xem mục 5.4).

### 1.6. Vì sao cần cấu trúc dữ liệu chuyên biệt

| Cách làm | Cập nhật đoạn | Truy vấn đoạn | Nhận xét |
|---|---|---|---|
| Mảng thường | `O(R−L+1)` | `O(R−L+1)` | đơn giản, nhưng `O(N)` mỗi thao tác |
| Mảng tổng tiền tố | `O(N)` | `O(1)` | truy vấn cực nhanh nhưng cập nhật phải dựng lại |
| Cây phân đoạn + lazy | `O(log N)` | `O(log N)` | **cân bằng cả hai chiều** |
| Fenwick (BIT) kép | `O(log N)` | `O(log N)` | gọn hơn nhưng khó mở rộng sang min/max/affine |

**[NHÓM TỰ VIẾT]** So sánh chi tiết hơn với Fenwick tree hai mảng (kỹ thuật
"range update range query" bằng hai BIT), nêu rõ vì sao chuyên đề chọn Segment
Tree: tính tổng quát của phép gộp và khả năng mở rộng sang gán đoạn, min/max,
affine — những thứ Fenwick không làm được.

---

## Mục 2. Định nghĩa ký hiệu

Mọi ký hiệu dưới đây được định nghĩa **trước khi sử dụng**.

| Ký hiệu | Định nghĩa |
|---|---|
| `N` | số phần tử của mảng gốc |
| `Q` | số thao tác |
| `v` | chỉ số một nút trong mảng biểu diễn cây; nút gốc là `v = 1` |
| `[tl, tr]` | đoạn chỉ số (0-based, đóng hai đầu) mà nút `v` quản lý |
| `tm` | điểm chia, `tm = tl + ⌊(tr − tl)/2⌋` |
| `[l, r]` | đoạn của thao tác đang xét |
| `len` | số phần tử của một đoạn, `len = tr − tl + 1` |
| `tree[v]` | giá trị gộp (ở đây là **tổng**) của đoạn mà nút `v` quản lý |
| `lazy[v]` | thẻ hoãn: lượng cộng đã áp cho `tree[v]` nhưng **chưa** áp cho hai con |
| `Value` | kiểu của giá trị gộp — *tổng của cả đoạn* |
| `Tag` | kiểu của giá trị cập nhật — *lượng cộng cho một phần tử* |
| `⊕` | phép gộp hai giá trị con thành giá trị cha; ở đây `⊕` là phép cộng |
| `e` | phần tử trung hòa của `⊕`; ở đây `e = 0` |
| `h` | chiều cao cây, `h = ⌈log₂ N⌉` |

**Điểm cần nhấn mạnh.** `Value` và `Tag` tuy cùng biểu diễn bằng `long long`
nhưng **khác nhau về ngữ nghĩa**. Đó là lý do trong hàm `apply` phải nhân thẻ
với độ dài đoạn: cộng `t` vào `len` phần tử làm tổng tăng thêm `t · len`, chứ
không phải `t`. Nhầm lẫn hai khái niệm này là nguồn lỗi phổ biến nhất khi cài
lazy propagation.

---

## Mục 3. Lý thuyết trọng tâm

### 3.1. Mỗi nút đại diện một đoạn

Cây phân đoạn được định nghĩa đệ quy trên đoạn `[0, N−1]`:

* nút gốc quản lý toàn bộ `[0, N−1]`;
* nút quản lý `[tl, tr]` với `tl < tr` có hai con quản lý `[tl, tm]` và
  `[tm+1, tr]`, trong đó `tm = tl + ⌊(tr − tl)/2⌋`;
* nút quản lý `[tl, tl]` là **lá**, ứng với đúng một phần tử của mảng gốc.

Tập các đoạn do các nút quản lý tạo thành một **phân hoạch phân cấp** của
`[0, N−1]`.

### 3.2. Phép gộp và cấu trúc đại số

Phép gộp `⊕` phải có **tính kết hợp** và có **phần tử trung hòa** `e` — tức là
`(Value, ⊕, e)` phải là một **monoid**. Với bài toán này `⊕` là phép cộng và
`e = 0`.

Tính kết hợp là điều kiện đủ để giá trị của một nút cha tính được từ hai con
theo bất kỳ thứ tự lồng nào, nhờ đó truy vấn có thể chia đoạn `[l, r]` thành
`O(log N)` đoạn con rời nhau rồi gộp lại.

**[NHÓM TỰ VIẾT]** Nêu thêm ví dụ các monoid khác dùng được ngay với cùng khung
cài đặt: `(min, +∞)`, `(max, −∞)`, `(gcd, 0)`, `(ma trận, nhân ma trận, I)`.

### 3.3. Chiều cao `O(log N)` và số nút chạm tới

**Mệnh đề.** Cây có chiều cao `h = ⌈log₂ N⌉`.

**Mệnh đề.** Với mỗi đoạn `[l, r]`, phép đệ quy chạm tới `O(log N)` nút.

*Ý tưởng chứng minh.* Ở mỗi mức của cây, các nút "giao một phần" với `[l, r]`
nhiều nhất là 2 (một ở phía biên trái, một ở phía biên phải). Mọi nút khác hoặc
nằm ngoài `[l, r]` (dừng ngay), hoặc nằm trọn trong `[l, r]` (dừng ngay sau khi
`apply`). Vậy tổng số nút chạm tới không quá `4h + O(1) = O(log N)`.

**[NHÓM TỰ VIẾT]** Viết chứng minh đầy đủ bằng quy nạp theo mức của cây, kèm
hình vẽ minh họa hai "biên" của đoạn truy vấn.

### 3.4. Vì sao mảng `4N` là cận an toàn phổ dụng

Với `N` không phải lũy thừa của 2, cây **không** phải cây nhị phân hoàn chỉnh:
một số nhánh sâu hơn nhánh khác. Nếu đánh số nút theo quy ước heap
(`con(v) = 2v` và `2v+1`), chỉ số lớn nhất có thể chạm tới nhỏ hơn `2^(h+1)`,
mà `2^(h+1) ≤ 4N` với mọi `N ≥ 1`. Do đó cấp phát `4N` ô là an toàn.

> **Lưu ý kỹ thuật (đúng theo yêu cầu của đề).** **Không** được gọi Segment Tree
> là "cây nhị phân hoàn toàn trong mọi trường hợp". Điều thật sự quan trọng là
> **phân hoạch đoạn** và **chiều cao logarit**, chứ không phải tính hoàn chỉnh
> của cây.

Có thể giảm bộ nhớ xuống `2·2^⌈log₂ N⌉` bằng cách đệm mảng lên lũy thừa của 2,
hoặc dùng cây phân đoạn lặp (iterative) chỉ cần `2N` ô — nhưng bản lặp khó cài
lazy propagation hơn đáng kể.

### 3.5. Lazy propagation: lưu phép cập nhật chưa đẩy xuống con

Ý tưởng cốt lõi: khi một nút nằm **trọn** trong đoạn cập nhật, ta cập nhật giá
trị gộp của nó **ngay lập tức** và **ghi nhớ** phép cập nhật vào `lazy[v]` thay
vì đi tiếp xuống toàn bộ cây con. Phép cập nhật chỉ được "trả nợ" khi nào thật
sự cần đi xuống con.

### 3.6. Bất biến giữa giá trị nút và nhãn lazy

Đây là phần **quan trọng nhất** của toàn chuyên đề.

* **(I1)** `tree[v]` là giá trị gộp **đúng** của đoạn mà nút `v` quản lý, đã bao
  gồm mọi cập nhật đã tác động lên `v`, **kể cả** `lazy[v]`.
* **(I2)** `lazy[v]` là cập nhật đã được áp cho `tree[v]` nhưng **chưa** được áp
  cho hai cây con của `v`.

**Hệ quả trực tiếp.** Đọc `tree[v]` **luôn** hợp lệ, không cần push_down trước.
Chỉ khi cần **đi xuống con** ta mới phải gọi `push_down` để khôi phục (I1) cho
các con.

Cần chứng minh cả ba thao tác `build`, `update_range`, `query_range` đều **bảo
toàn** (I1) và (I2).

**[NHÓM TỰ VIẾT]** Viết chứng minh bảo toàn bất biến cho từng thao tác. Gợi ý
cấu trúc: quy nạp theo chiều cao cây con, xét riêng ba trường hợp ở mục 3.7.

### 3.7. Ba trường hợp của đệ quy

Với nút `v` quản lý `[tl, tr]` và thao tác trên `[l, r]`:

| Trường hợp | Điều kiện | Hành động |
|---|---|---|
| **Nằm ngoài** | `r < tl` hoặc `tr < l` | dừng ngay; truy vấn trả `e` |
| **Phủ toàn bộ** | `l ≤ tl` và `tr ≤ r` | cập nhật: `apply` rồi **dừng**. Truy vấn: trả `tree[v]` |
| **Giao một phần** | còn lại | `push_down`, đệ quy hai con, rồi `pull` (với cập nhật) |

Trường hợp **phủ toàn bộ** chính là nơi lazy propagation tiết kiệm: ta dừng ở
một nút thay vì đi xuống toàn bộ cây con bên dưới nó.

### 3.8. Ba thao tác nguyên thủy

```
apply(v, len, t):        # áp thẻ t lên nút v quản lý len phần tử
    tree[v] += t * len   # tổng của len phần tử tăng thêm t * len
    lazy[v] += t         # hợp thành thẻ: phép cộng giao hoán nên cộng dồn

push_down(v, tl, tm, tr):
    if lazy[v] == 0: return
    apply(2v,   tm - tl + 1, lazy[v])   # con trái giữ tm - tl + 1 phần tử
    apply(2v+1, tr - tm,     lazy[v])   # con phải giữ tr - tm phần tử
    lazy[v] = 0

pull(v):
    tree[v] = tree[2v] ⊕ tree[2v+1]
```

Chú ý độ dài hai nửa được truyền **tường minh**, không giả định `len/2`, vì khi
`N` không là lũy thừa của 2 hai nửa có thể lệch nhau một phần tử.

### 3.9. Mã giả

```
build(v, tl, tr):
    if tl == tr:
        tree[v] ← a[tl]
    else:
        tm ← tl + (tr - tl) / 2
        build(2v,   tl,     tm)
        build(2v+1, tm + 1, tr)
        pull(v)

update_range(v, tl, tr, l, r, t):
    if r < tl or tr < l:            return              # (1) nằm ngoài
    if l <= tl and tr <= r:                             # (2) phủ toàn bộ
        apply(v, tr - tl + 1, t);   return
    tm ← tl + (tr - tl) / 2                             # (3) giao một phần
    push_down(v, tl, tm, tr)
    update_range(2v,   tl,     tm, l, r, t)
    update_range(2v+1, tm + 1, tr, l, r, t)
    pull(v)

query_range(v, tl, tr, l, r):
    if r < tl or tr < l:            return e            # (1) nằm ngoài
    if l <= tl and tr <= r:         return tree[v]      # (2) phủ toàn bộ
    tm ← tl + (tr - tl) / 2                             # (3) giao một phần
    push_down(v, tl, tm, tr)
    return query_range(2v,   tl,     tm, l, r)
         ⊕ query_range(2v+1, tm + 1, tr, l, r)
```

### 3.10. Vì sao thuật toán đúng

Lập luận gồm ba phần, cần trình bày đầy đủ trong báo cáo:

1. **Tính đúng của phân hoạch.** Đoạn `[l, r]` được phân tích thành hợp của các
   đoạn nút **rời nhau** thuộc trường hợp "phủ toàn bộ". Vì `⊕` có tính kết hợp,
   gộp chúng theo bất kỳ thứ tự nào cũng cho cùng kết quả.
2. **Tính đúng của lazy.** Nhờ (I1), giá trị đọc ở trường hợp "phủ toàn bộ" đã
   đúng mà không cần biết bên dưới còn nợ gì. Nhờ (I2) và `push_down`, mỗi khi
   đi xuống con thì con cũng đã đúng.
3. **Tính đúng của hợp thành thẻ.** Với phép cộng, hợp thành hai thẻ là phép
   cộng — giao hoán và kết hợp — nên thứ tự đẩy xuống không ảnh hưởng kết quả.
   *(Với phép gán hoặc affine ở mục 8 thì thứ tự **có** ảnh hưởng, và luật hợp
   thành phải được định nghĩa cẩn thận.)*

**[NHÓM TỰ VIẾT]** Viết đầy đủ ba phần trên thành chứng minh chặt chẽ.

### 3.11. Bảng độ phức tạp

| Thao tác | Thời gian | Bộ nhớ thêm | Khớp với mã nguồn |
|---|---|---|---|
| `build` | `O(N)` | `O(1)` ngoài cây | `build_rec`, thăm mỗi nút đúng một lần |
| `update_range` | `O(log N)` | `O(log N)` ngăn xếp đệ quy | `update_rec`, `O(log N)` nút chạm tới |
| `query_range` | `O(log N)` | `O(log N)` ngăn xếp đệ quy | `query_rec` |
| Toàn bộ cấu trúc | — | `8N` ô 64-bit = **64 byte/phần tử** | `4N` ô `tree_` + `4N` ô `lazy_` |

Con số 64 byte/phần tử được **xác nhận bằng thực nghiệm** ở Bảng 3 của
`results/results.md`.

---

## Mục 4. Yêu cầu cài đặt và cách đáp ứng

| Yêu cầu của đề | Nơi đáp ứng |
|---|---|
| Đóng gói lớp `SegmentTree` | `src/segment_tree.hpp`, lớp `st::SegmentTree` |
| có `build` | `SegmentTree::build` |
| có `update_range` | `SegmentTree::update_range` |
| có `query_range` | `SegmentTree::query_range` |
| có `apply` | `SegmentTree::apply` (private) |
| có `push_down` | `SegmentTree::push_down` (private) |
| Tách rõ kiểu giá trị cây và giá trị cập nhật | `using Value` và `using Tag`, kèm chú thích ngữ nghĩa |
| Dùng 64-bit cho tổng | `Value = long long`, kiểm chứng ở test `08_large_values_64bit` |

### 4.1. Giao diện công khai

```cpp
class SegmentTree {
public:
    using Value = long long;   // giá trị gộp tại nút: TỔNG của cả đoạn
    using Tag   = long long;   // giá trị cập nhật hoãn: lượng cộng cho MỘT phần tử

    SegmentTree() = default;
    explicit SegmentTree(const std::vector<Value>& a);

    void  build(const std::vector<Value>& a);   // O(N)
    void  update_range(int l, int r, Tag v);    // O(log N)
    Value query_range(int l, int r);            // O(log N)

    int   size() const noexcept;
    bool  empty() const noexcept;
    Value at(int i);                            // tiện ích cho kiểm thử
    std::vector<Value> to_vector();             // trải phẳng cây, cho kiểm thử
    std::size_t memory_bytes() const noexcept;  // cho phần thực nghiệm
};
```

**Quy ước chỉ số.** Giao diện công khai dùng chỉ số **0-based**, đoạn **đóng**
`[l, r]` với `0 ≤ l ≤ r < N`. Việc chuyển từ 1-based (định dạng dữ liệu vào)
sang 0-based được đặt ở **tầng nhập/xuất** trong `src/main.cpp`, không lẫn vào
cấu trúc dữ liệu.

### 4.2. Vì sao `query_range` không phải hàm `const`

Vì `query_range` có thể phải gọi `push_down`, tức là **thay đổi** trạng thái nội
tại của cây. Đây là một hệ quả trực tiếp của lazy propagation và cần được nêu rõ
trong báo cáo. (Nếu muốn `const`, phải khai báo `tree_`/`lazy_` là `mutable`,
nhưng như vậy làm mờ ý nghĩa của `const` nên cài đặt này không dùng.)

### 4.3. Những chi tiết cài đặt dễ sai

| Chi tiết | Sai lầm thường gặp | Cách cài đặt đúng trong dự án |
|---|---|---|
| Tính điểm chia | `tm = (tl + tr) / 2` tràn khi chỉ số lớn | `tm = tl + (tr - tl) / 2` |
| Độ dài hai nửa | giả định mỗi nửa `len/2` | truyền `tm - tl + 1` và `tr - tm` tường minh |
| `apply` cho tổng | `tree[v] += t` (quên nhân độ dài) | `tree[v] += t * len` |
| Phần tử trung hòa | dùng `LLONG_MAX` cho phép cộng | `VALUE_IDENTITY = 0` |
| Trộn hai phép gộp | `build` cộng nhưng `query` lấy min | một hàm `combine` duy nhất |
| Kích thước mảng | cấp phát `2N` | cấp phát `4N` |

Hàng cuối cùng và hàng "trộn hai phép gộp" chính là **lỗi có thật đã được phát
hiện trong bản nháp đầu tiên** của nhóm — xem mục 5.2.

### 4.4. Cấu trúc chương trình

```
src/segment_tree.hpp      lớp chính (header-only)
src/segment_tree_ext.hpp  hai biến thể mở rộng
src/naive.hpp             lời giải mảng thường, cùng giao diện công khai
src/main.cpp              tầng nhập/xuất + điều phối, độc lập với cấu trúc dữ liệu
src/bench.cpp             chương trình đo thời gian
```

Việc `NaiveArray` có **cùng giao diện công khai** với `SegmentTree` cho phép
`src/main.cpp` dùng chung một khuôn hàm `run<Engine>` cho cả hai, nên khi đối
chiếu ta chắc chắn hai bên nhận **cùng một chuỗi thao tác**.

---

## Mục 5. Kiểm thử

### 5.1. Chiến lược tổng thể

Bốn tầng bảo vệ, chạy bằng một lệnh `.\scripts\run_tests.ps1`:

| Tầng | Công cụ | Phát hiện được gì |
|---|---|---|
| Kiểm thử đơn vị | `tests/unit_tests.cpp` | lỗi logic ở các trường hợp cụ thể, kiểm tra được bằng tay |
| Kiểm tra tràn số | bản dịch `-DST_CHECK_OVERFLOW` | vi phạm điều kiện an toàn 64-bit |
| Test cố định | `tests/cases/*.in` + `.out` | hồi quy: bảo đảm sửa chỗ này không hỏng chỗ khác |
| Stress test | `tests/stress_test.cpp` | lỗi ở tổ hợp trạng thái mà con người không nghĩ ra |

### 5.2. Bốn nhóm kiểm thử đơn vị

Tổng cộng **71 khẳng định**, tất cả đều đạt.

**(a) Test cơ bản** — mảng `[1, 2, 8, 9, 3]`, mọi kết quả **tính tay được**:
tổng toàn mảng `= 23`, `a[2..4] = 19`, sau khi cộng `5` vào `[2,4]` thì mảng
thành `[1, 7, 13, 14, 3]` nên tổng `= 38`.

**(b) Test biên**

| Trường hợp biên | Vì sao quan trọng |
|---|---|
| `N = 1` | cây chỉ có một nút; kiểm tra đường đi lá đặc biệt |
| `N = 0` | cây rỗng không được làm sập chương trình |
| `N` không là lũy thừa của 2 (3, 5, 6, 7, 9, 11, 13, 100, 1000, 4097) | hai nửa lệch nhau, dễ lộ lỗi tính độ dài |
| cập nhật toàn đoạn `[0, N−1]` | thẻ dừng ngay ở gốc |
| cập nhật một điểm (`l = r`) | đường đi sâu nhất |
| giá trị âm | dễ lộ lỗi khởi tạo phần tử trung hòa |
| `V = 0` (cập nhật rỗng) | không được làm hỏng bất biến |
| tổng `2.2 · 10^15`, phần tử `1.1 · 10^10` | vượt xa phạm vi 32-bit |
| cộng rồi trừ triệt tiêu | kiểm tra dấu và tính giao hoán của thẻ |

**(c) Test đối kháng** — các mẫu cố ý làm khó lazy propagation:

1. **Test hồi quy cho lỗi "cây lai"** (xem 5.3): với `N = 5`, đoạn `[2,3]`
   (0-based) đúng bằng **một nút trong** của cây, nên bản cài đặt lỗi trả về giá
   trị của nút đó. Test yêu cầu tổng `= 17` **và** min `= 8`, buộc hai phép gộp
   phải tách bạch.
2. **Ép `push_down` tối đa**: `N = 1023`, cập nhật toàn đoạn 20 lần rồi truy vấn
   **từng phần tử một** — mỗi truy vấn buộc đẩy thẻ xuống hết chiều cao cây.
3. **Đoạn lồng nhau thu hẹp dần** `[0, N−1] ⊃ [1, N−2] ⊃ …` rồi truy vấn mọi
   tiền tố.
4. **Đoạn lệch cực đoan**: cập nhật toàn bộ các tiền tố, rồi toàn bộ các hậu tố.
5. **Kiểm tra rò rỉ qua ranh giới**: cập nhật 100 lần lên nửa trái, truy vấn nửa
   phải phải vẫn bằng `0`.
6. **Hai đoạn giao nhau đúng một phần tử**: phần tử giao phải nhận **cả hai** cập
   nhật.

**(d) Test ngẫu nhiên đối chiếu** — 300 vòng với `N ≤ 60` và 200 thao tác mỗi
vòng, cộng một vòng lớn `N = 5000` với giá trị tới `10^9` và 3000 thao tác. Mọi
kết quả đều so với `NaiveArray`, và trạng thái **cuối cùng của cả mảng** cũng
được so, không chỉ các kết quả truy vấn.

### 5.3. Lỗi thật đã phát hiện trong bản nháp đầu tiên

Bản nháp đầu tiên của nhóm mắc một lỗi đáng ghi lại vì nó rất khó thấy:

```cpp
void build(int v, int l, int r) {
    ...
    t[v] = t[2*v] + t[2*v + 1];    // GỘP BẰNG TỔNG
}
void update(int v, int l, int r, int pos, int val) {
    ...
    t[v] = min(t[2*v], t[2*v+1]);  // GỘP BẰNG MIN  ← không nhất quán
}
ll query(int v, int tl, int tr, int l, int r) {
    if (l > r) return LLONG_MAX;   // phần tử trung hòa của MIN
    if (tl == l && tr == r) return t[v];
    ...
    return min(s1, s2);            // GỘP BẰNG MIN
}
```

`build` dựng cây **tổng**, nhưng `query` lại gộp bằng **min** trên các nút chứa
tổng. Lỗi này **không lộ ra** trên nhiều dữ liệu nhỏ: nếu đoạn truy vấn tình cờ
bị phân tích thành các nút lá thì min trên các lá vẫn đúng. Nó chỉ lộ khi đoạn
truy vấn phủ trọn một **nút trong**. Với `a = [1, 2, 8, 9, 3]`, truy vấn min trên
`[3, 4]` (1-based) rơi đúng vào một nút trong có giá trị `8 + 9 = 17`, nên trả
về `17` thay vì `8`.

**Bài học rút ra:** phép gộp phải nằm ở **một chỗ duy nhất** trong mã. Cài đặt
hiện tại có đúng một hàm `combine`, nên loại hẳn khả năng lặp lại lỗi này. Đã bổ
sung một test hồi quy chuyên cho tình huống này.

Ngoài ra bản nháp còn thiếu: lazy propagation (chỉ có cập nhật một điểm), thiếu
`#include <climits>` cho `LLONG_MAX`, và bộ sinh dữ liệu sinh sai định dạng nên
không thể đối chiếu tự động.

**[NHÓM TỰ VIẾT]** Bổ sung mô tả quá trình nhóm phát hiện lỗi này.

### 5.4. Kiểm tra tràn 64-bit

Khi dịch với `-DST_CHECK_OVERFLOW`, hàm `apply` tính trước kết quả bằng kiểu
128-bit và dừng chương trình nếu vượt phạm vi `long long`:

```cpp
W total = static_cast<W>(tree_[v]) + static_cast<W>(t) * static_cast<W>(len);
assert(total >= kI64Min && total <= kI64Max && "TRAN 64-bit: ...");
```

Toàn bộ 71 khẳng định được chạy lại với bản dịch này, xác nhận không có test nào
vô tình vi phạm điều kiện an toàn ở mục 1.5.

### 5.5. Bộ test cố định

12 tệp `.in`/`.out` trong `tests/cases/`, chia 3 nhóm: **cơ bản** (ví dụ tính
tay được), **biên** (`N = 1`, `N` không phải lũy thừa 2, giá trị âm, cập nhật
`0`, tràn 32-bit, `N = 100000`), **đối kháng** (toàn đoạn, một điểm, tiền tố,
chồng lấn ngẫu nhiên, xen kẽ liên tục — cả ở quy mô nhỏ lẫn `N = 100000`). Một
vài đại diện tiêu biểu:

| Tệp | Loại | Điểm nhấn |
|---|---|---|
| `01_basic_sample` | cơ bản | ví dụ trong README, tính tay được |
| `02_single_element` | biên | `N = 1` |
| `08_large_values_64bit` | biên | kết quả `2 · 10^10`, vượt 32-bit |
| `10_adversarial_full` | đối kháng | mọi thao tác phủ `[1, N]` |
| `16_boundary_n_large` | biên | `N = 100000` cố định |
| `19_adversarial_alternating_n100000` | đối kháng | `N = 100000`, xen kẽ liên tục toàn đoạn / một điểm |

Danh sách đầy đủ 12 tệp: xem thư mục `tests/cases/`.

**Cách xác thực đáp án chuẩn.** `tests/make_expected.ps1` chạy mỗi tệp `.in`
bằng **cả hai** cài đặt và chỉ ghi ra `.out` khi hai kết quả trùng nhau. Thêm
vào đó, kết quả của các test cơ bản/biên đầu tiên đã được **tính tay** và đối
chiếu độc lập — cả ba nguồn đều khớp, ví dụ:

| Test | Kết quả (đã kiểm tra bằng ba cách độc lập) |
|---|---|
| `01_basic_sample` | 23, 19, 38, 34, 13 |
| `02_single_element` | 42, −58, 999999942 |
| `08_large_values_64bit` | 5000000000, 20000000000, 4000000000 |

### 5.6. Stress test

`tests/stress_test.cpp` sinh ngẫu nhiên hàng nghìn kịch bản và chạy song song
hai cài đặt. Điểm thiết kế đáng chú ý:

* Hạt giống mỗi vòng suy ra từ `(seed, round)` nên **mọi vòng đều tái lập được**.
* Cứ 8 thao tác lại **ép** một dạng biên: toàn đoạn, một điểm, tiền tố, hậu tố.
* Khi phát hiện sai lệch, chương trình **ghi kịch bản lỗi ra `stress_fail.in`**
  để có thể gỡ lỗi lại y hệt.
* Không chỉ so các kết quả truy vấn mà còn so **toàn bộ mảng ở trạng thái cuối**.

**Kết quả:** `3000` vòng × `500` thao tác ≈ **1,5 triệu thao tác ngẫu nhiên**,
không có sai lệch nào.

### 5.7. Bảng kết quả kiểm thử

| Bước | Nội dung | Kết quả |
|---|---|---|
| 1 | Kiểm thử đơn vị (bản tối ưu) | **71 đạt / 0 hỏng** |
| 2 | Kiểm thử đơn vị (bản `ST_CHECK_OVERFLOW`) | **71 đạt / 0 hỏng** |
| 3 | 12 test cố định | **12 đạt / 0 hỏng** |
| 4 | Stress test 3000 vòng | **không có sai lệch** |

Nhật ký đầy đủ: `results/test_log.txt`.

---

## Mục 6. Thực nghiệm

### 6.1. Môi trường chạy

Xem `results/environment.md` (sinh tự động). Tóm tắt:

| Hạng mục | Giá trị |
|---|---|
| Hệ điều hành | Windows 11 Home Single Language, build 26200 |
| CPU | AMD Ryzen AI 5 330 w/ Radeon 820M, 4 nhân / 8 luồng |
| RAM | 15,1 GB |
| Trình biên dịch | g++ 16.1.0 (MSYS2 UCRT64) |
| Cờ dịch | `-std=c++17 -O2 -DNDEBUG -static` |
| Đồng hồ đo | `std::chrono::steady_clock` |
| Đơn vị | ms; `ns/op` = nano giây mỗi thao tác |

### 6.2. Phương pháp đo

* Chuỗi thao tác được **sinh trước và nạp sẵn vào bộ nhớ**, nên thời gian đo
  không lẫn chi phí sinh số ngẫu nhiên hay nhập/xuất.
* Mỗi cấu hình chạy **5 lần**, báo cáo giá trị **nhỏ nhất** (ít nhiễu nhất từ hệ
  điều hành), đồng thời lưu cả giá trị trung bình vào CSV.
* `checksum` là tổng của mọi kết quả truy vấn: vừa chặn trình tối ưu loại bỏ
  vòng lặp, vừa **xác nhận cây phân đoạn và mảng thường cho cùng kết quả**.
* Năm kiểu tải: `mixed` (50/50), `update`, `query`, `point` (`L = R`), `full`
  (mọi thao tác phủ `[1, N]`).

### 6.3. Kết quả — thời gian theo kích thước `N`

| `N` | `Q` | Segment Tree (ms) | ns/op | Mảng thường (ms) | ns/op | Tăng tốc |
|---:|---:|---:|---:|---:|---:|---:|
| 1 000 | 200 000 | 65,8 | 329,0 | 39,6 | 197,9 | **0,6×** |
| 10 000 | 200 000 | 90,2 | 451,2 | 321,0 | 1 605,1 | **3,6×** |
| 100 000 | 200 000 | 201,1 | 1 005,5 | 3 167,6 | 15 838,0 | **15,8×** |
| 1 000 000 | 200 000 | 323,0 | 1 615,1 | không đo (quá chậm) | — | — |

*Hình 1 (`report/figures/fig1_scaling.svg`)* — biểu đồ hai trục loga.

**Nhận xét quan trọng và trung thực:** ở `N = 1000`, mảng thường **nhanh hơn**
cây phân đoạn (0,6×). Điều này hoàn toàn hợp lý: `O(N)` với hằng số nhỏ và truy
cập bộ nhớ liên tục vẫn thắng `O(log N)` với hằng số lớn và truy cập rải rác khi
`N` nhỏ. Cây phân đoạn chỉ thật sự có lợi từ khoảng `N ≥ 5000`. **Không được
phóng đại khả năng của thuật toán.**

### 6.4. Kết quả — kiểm chứng `O(log N)`

*Hình 2 (`report/figures/fig2_logn.svg`)* vẽ `ns/op` theo `log₂ N` với trục tung
**tuyến tính**. Hồi quy tuyến tính cho

```
ns/op ≈ 132,8 · log₂ N − 1135,5      (R² = 0,936)
```

Việc các điểm gần một đường thẳng khi vẽ theo `log₂ N` là bằng chứng thực nghiệm
cho độ phức tạp `O(log N)`.

**[NHÓM TỰ VIẾT]** Giải thích vì sao `R²` chưa đạt `1,0`: hằng số ẩn không thật
sự là hằng vì **hiệu ứng bộ nhớ đệm** thay đổi theo `N`. Ở `N = 10^6`, cây chiếm
khoảng 61 MB, vượt xa bộ nhớ đệm cấp ba, nên mỗi lần đi xuống một mức là một lần
"trượt" bộ nhớ đệm. Có thể bổ sung số liệu bằng cách chạy thêm các mốc `N` trung
gian.

### 6.5. Kết quả — thời gian theo kiểu tải

| `N` | mixed | update | query | point | **full** |
|---:|---:|---:|---:|---:|---:|
| 1 000 | 329,0 | 391,8 | 218,6 | 157,9 | **9,0** |
| 10 000 | 451,2 | 590,4 | 302,4 | 206,8 | **8,8** |
| 100 000 | 1 005,5 | 1 009,7 | 531,3 | 371,0 | **8,8** |
| 1 000 000 | 1 615,1 | 1 875,6 | 1 024,6 | 694,8 | **9,2** |

*(đơn vị: ns mỗi thao tác. Hình 3: `report/figures/fig3_workloads.svg`)*

**Đây là số liệu thuyết phục nhất của toàn bộ chuyên đề.** Ở kiểu tải `full`,
thời gian mỗi thao tác **gần như không đổi (≈ 9 ns) dù `N` tăng 1000 lần**, vì
mọi thao tác dừng ngay tại nút gốc: `apply` một lần rồi thôi. Đó chính xác là
lợi ích mà lazy propagation mang lại. Trong khi đó mảng thường ở cùng kiểu tải
mất `4238 ns/op` tại `N = 10 000` — chậm hơn **480 lần**.

### 6.6. Kết quả — thời gian dựng cây và bộ nhớ

| `N` | Thời gian build (ms) | Bộ nhớ cây (MB) | Byte/phần tử |
|---:|---:|---:|---:|
| 1 000 | 0,102 | 0,06 | **64,0** |
| 10 000 | 0,132 | 0,61 | **64,0** |
| 100 000 | 4,106 | 6,10 | **64,0** |
| 1 000 000 | 37,314 | 61,04 | **64,0** |

Con số 64 byte/phần tử **khớp chính xác** với dự đoán lý thuyết ở mục 3.11:
`4N` ô cho `tree_` + `4N` ô cho `lazy_`, mỗi ô 8 byte.

### 6.7. Kết quả — đo trọn gói trên tệp dữ liệu thật

| Tệp dữ liệu | `N` | `Q` | Cài đặt | Tổng (ms) | Build (ms) | Thao tác (ms) |
|---|---:|---:|---|---:|---:|---:|
| `small_n1000_q1000` | 1 000 | 1 000 | segtree | 33,5 | 0,06 | 0,36 |
| `small_n1000_q1000` | 1 000 | 1 000 | naive | 28,2 | 0,00 | 0,33 |
| `medium_n50000_q50000` | 50 000 | 50 000 | segtree | 81,9 | 1,63 | **39,5** |
| `medium_n50000_q50000` | 50 000 | 50 000 | naive | 467,0 | 0,03 | **424,7** |
| `large_n1000000_q1000000` | 10^6 | 10^6 | segtree | 2 034,1 | 29,47 | 1 786,9 |
| `adv_full_n200000_q200000` | 200 000 | 200 000 | segtree | 81,5 | 6,25 | **14,1** |
| `adv_point_n200000_q200000` | 200 000 | 200 000 | segtree | 216,3 | 5,21 | 143,0 |
| `adv_prefix_n200000_q200000` | 200 000 | 200 000 | segtree | 211,4 | 5,73 | 137,6 |

**Nhận xét.** Bài toán quy mô `N = Q = 10^6` được giải trong khoảng **2 giây
trọn gói**, trong đó chỉ 1,79 giây là xử lý — phần còn lại là đọc tệp 25 MB và
ghi kết quả. Dữ liệu `adv_full` chỉ mất 14 ms cho 200 000 thao tác, một lần nữa
xác nhận lợi ích của lazy propagation.

---

## Mục 7. Ứng dụng minh họa

### 7.1. Mô hình hóa bài toán thực tế

Bài toán "cộng vào đoạn, hỏi tổng đoạn" mô hình hóa trực tiếp việc **duy trì
tổng trên chuỗi dữ liệu thời gian có cập nhật theo khoảng và truy vấn tức thì**.

**Ví dụ cụ thể.** Một hệ thống đặt phòng khách sạn ghi số khách theo từng ngày
trong năm (`N = 365`, hoặc `N = 3650` cho mười năm):

| Nghiệp vụ | Thao tác |
|---|---|
| Một đơn đặt phòng từ ngày `L` đến ngày `R` cho `V` khách | `1 L R V` |
| Hủy đơn đó | `1 L R -V` |
| "Tổng số đêm khách trong quý II là bao nhiêu?" | `2 L R` |

Nếu thay `combine` bằng `max` (bằng `SegmentTreeAssignAdd` ở mục 8), cùng cấu
trúc đó trả lời được **"ngày đông khách nhất trong khoảng là bao nhiêu khách?"**
— hữu ích để kiểm tra ràng buộc sức chứa trước khi nhận đơn mới.

### 7.2. Các miền ứng dụng khác

**[NHÓM TỰ VIẾT]** Mở rộng thêm một hoặc hai ví dụ, ví dụ:
* Chuỗi đo cảm biến theo thời gian, hiệu chỉnh (calibration) theo khoảng thời gian.
* Bảng tính: cộng một lượng vào một dải ô rồi hỏi tổng của dải khác.
* Xử lý ảnh một chiều: điều chỉnh độ sáng theo dải rồi hỏi tổng cường độ.

### 7.3. Giới hạn của mô hình — **không phóng đại khả năng thuật toán**

Đây là mục **bắt buộc** theo checklist của đề.

| Giới hạn | Giải thích |
|---|---|
| Phép gộp phải có tính kết hợp | không dùng được cho "phần tử xuất hiện nhiều nhất" |
| Thẻ phải hợp thành được với kích thước hằng số | không dùng được cho phép cập nhật tùy ý |
| Chỉ hiệu quả trên **một chiều** | dữ liệu nhiều chiều cần cây phân đoạn 2D, chi phí `O(log² N)` |
| Kích thước mảng cố định | thêm/xóa phần tử ở giữa cần cây cân bằng hoặc cây phân đoạn động |
| Không thay thế cơ sở dữ liệu | không có tính bền vững, giao tác, hay truy vấn tùy ý |
| Chỉ có lợi khi `N` đủ lớn | ở `N = 1000` mảng thường còn nhanh hơn (mục 6.3) |
| Với dữ liệu chỉ đọc | mảng tổng tiền tố đơn giản hơn và nhanh hơn nhiều |

---

## Mục 8. Mở rộng đã cài đặt

Cả hai nằm trong `src/segment_tree_ext.hpp` và đều được đối chiếu ngẫu nhiên với
mảng thường trong `tests/unit_tests.cpp`.

### 8.1. `SegmentTreeAssignAdd` — gán đoạn + cộng đoạn, truy vấn tổng/min/max

* `Value` là bộ ba `{sum, min, max}`; phần tử trung hòa là `{0, +∞, −∞}`.
* `Tag` là bộ ba `{has_assign, assign_val, add_val}`, ngữ nghĩa: "nếu có gán thì
  **gán trước**, sau đó **cộng**".
* **Hợp thành thẻ không giao hoán.** Áp thẻ `b` lên trên thẻ `a` đã có:

```
nếu b có phép gán  →  kết quả là b  (phép gán mới XÓA SẠCH mọi thứ trước đó)
ngược lại          →  giữ phép gán của a, cộng dồn phần add
```

Điểm mấu chốt: **mọi dãy thao tác gán/cộng đều rút gọn được về đúng dạng
`(gán rồi cộng)`**, nên thẻ vẫn có kích thước hằng số. Đây là lý do biến thể này
cài được với cùng độ phức tạp `O(log N)`.

Test kiểm chứng tính không giao hoán:

```
cộng 1000 vào [0,5] rồi gán 7 vào [0,5]  →  tổng = 6 · 7 = 42   (gán thắng)
```

### 8.2. `SegmentTreeAffine` — cập nhật affine `x ↦ a·x + b (mod p)`

* `Tag` là cặp `(a, b)`; thẻ đơn vị là `(1, 0)`.
* `apply` cho một nút có `len` phần tử: `sum ← a · sum + b · len (mod p)`.
* **Hợp thành:** áp `(a₁,b₁)` trước rồi `(a₂,b₂)` sau cho

```
x ↦ a₂·(a₁·x + b₁) + b₂ = (a₂a₁)·x + (a₂b₁ + b₂)
```

Test kiểm chứng thứ tự **có** ảnh hưởng:

```
x = 10, nhân 2 rồi cộng 3  →  23
x = 10, cộng 3 rồi nhân 2  →  26
```

Gán đoạn là trường hợp riêng `affine(0, x)`; cộng đoạn là `affine(1, x)`.

### 8.3. Các mở rộng **chưa** cài — hướng phát triển tiếp

**[NHÓM TỰ VIẾT]** Nếu còn thời gian, có thể bổ sung:

| Mở rộng | Ý tưởng cốt lõi | Độ phức tạp |
|---|---|---|
| Segment tree động (implicit) | chỉ tạo nút khi cần, cho miền chỉ số tới `10^18` | `O(log C)` bộ nhớ mỗi thao tác |
| Persistent segment tree | mỗi cập nhật tạo `O(log N)` nút mới, giữ lại mọi phiên bản cũ | `O(log N)` bộ nhớ mỗi cập nhật |
| Segment tree beats | hỗ trợ `chmin`/`chmax` theo đoạn | `O(log² N)` khấu hao |
| Segment tree 2D | truy vấn hình chữ nhật | `O(log² N)` |

---

## Mục 9. Kết luận

**[NHÓM TỰ VIẾT]** Gợi ý cấu trúc:

1. Đã cài đặt hoàn chỉnh cây phân đoạn với lazy propagation cho bài toán cập
   nhật đoạn + truy vấn tổng đoạn, đạt `O(log N)` cho cả hai thao tác.
2. Tính đúng đắn được bảo đảm bằng bốn tầng kiểm thử độc lập; **1,5 triệu thao
   tác ngẫu nhiên** đối chiếu với mảng thường không phát hiện sai lệch nào.
3. Thực nghiệm xác nhận độ phức tạp `O(log N)` (`R² = 0,936` khi hồi quy theo
   `log₂ N`) và mức tiêu tốn bộ nhớ 64 byte/phần tử đúng như phân tích lý thuyết.
4. Điểm cần trung thực: ở `N` nhỏ (dưới khoảng 5000) mảng thường vẫn nhanh hơn;
   lợi ích của cây phân đoạn chỉ rõ rệt từ `N ≥ 10^4`.
5. Bài học quan trọng nhất về mặt kỹ thuật: **phép gộp phải nằm ở một chỗ duy
   nhất trong mã**, và phải phân biệt rành mạch *giá trị của cây* với *giá trị
   của phép cập nhật*.

---

## Mục 10. Tài liệu tham khảo

1. Cormen, T. H., Leiserson, C. E., Rivest, R. L., Stein, C. *Introduction to
   Algorithms*, 4th ed. MIT Press, 2022.
2. Halim, S., Halim, F., Effendy, S. *Competitive Programming 4*, Book 2. Lulu, 2020.
3. cp-algorithms.com, *Segment Tree*.
   https://cp-algorithms.com/data_structures/segment_tree.html
4. Laaksonen, A. *Competitive Programmer's Handbook*, 2018, chương 28.
5. **[NHÓM TỰ VIẾT]** Bổ sung các nguồn khác mà nhóm thực sự đã đọc.

---

## Phụ lục A. Hướng dẫn biên dịch và chạy lại

Xem `README.md` ở thư mục gốc. Tóm tắt:

```powershell
.\scripts\build.ps1              # biên dịch
.\scripts\run_tests.ps1          # chạy toàn bộ kiểm thử
.\scripts\run_experiments.ps1    # chạy toàn bộ thực nghiệm
.\scripts\make_figures.ps1       # sinh lại hình minh họa từ số liệu
```

## Phụ lục B. Mã nguồn

* `src/segment_tree.hpp` — lớp chính
* `src/segment_tree_ext.hpp` — hai biến thể mở rộng
* `src/naive.hpp` — lời giải đối chiếu
* `src/main.cpp`, `src/bench.cpp`
* `tests/unit_tests.cpp`, `tests/stress_test.cpp`, `tests/generator.cpp`

## Phụ lục C. Nhật ký sử dụng AI

Xem `report/ai_usage_log.md`.
