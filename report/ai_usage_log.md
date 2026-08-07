# Nhật ký sử dụng AI / công cụ hỗ trợ

**Chuyên đề 6 — Cây phân đoạn (Segment Tree) và Lazy Propagation**

Tài liệu này khai báo đầy đủ việc sử dụng công cụ hỗ trợ theo yêu cầu của đề:
**mục đích**, **câu hỏi chính**, **phần đã kiểm chứng**, và **phần nhóm tự sửa**.

> **[NHÓM TỰ VIẾT]** Bổ sung các phiên làm việc khác của nhóm với công cụ hỗ
> trợ (nếu có), và ghi rõ thành viên nào phụ trách phiên nào. Phần dưới đây ghi
> lại phiên rà soát và tái cấu trúc mã nguồn.

---

## 1. Công cụ đã dùng

| Công cụ | Phiên bản / môi trường | Mục đích |
|---|---|---|
| Claude (Anthropic) qua Claude Code | phiên ngày 07/08/2026 | rà soát mã nguồn bản nháp, tái cấu trúc dự án, viết bộ kiểm thử và chương trình đo thời gian |
| g++ (MSYS2 UCRT64) | 16.1.0 | biên dịch, bật cảnh báo nghiêm ngặt |
| PowerShell | 5.1 | script biên dịch, chạy kiểm thử và thực nghiệm |

---

## 2. Mục đích sử dụng

1. **Rà soát tính đúng đắn** của bản cài đặt nháp (`_backup_original/main_original.cpp`).
2. **Tái cấu trúc** dự án theo đúng cấu trúc thư mục mà đề yêu cầu.
3. **Sinh bộ kiểm thử** đủ bốn nhóm: cơ bản, biên, đối kháng, ngẫu nhiên đối chiếu.
4. **Xây dựng khung đo thời gian** có thể tái lập và có ghi lại môi trường chạy.

Công cụ **không** được dùng để thay nhóm viết phần lý thuyết hay phần kết luận
của báo cáo; các mục đó do nhóm tự viết (xem các chỗ đánh dấu **[NHÓM TỰ VIẾT]**
trong `report/report.md`).

---

## 3. Các câu hỏi chính đã đặt ra

| # | Câu hỏi | Kết quả |
|---|---|---|
| 1 | Bản nháp `main.cpp` có đúng không? Nếu sai thì sai ở đâu? | Phát hiện lỗi "cây lai": `build` gộp bằng **tổng** nhưng `update`/`query` gộp bằng **min**. Xem mục 4.1. |
| 2 | Bản nháp còn thiếu gì so với yêu cầu của chuyên đề 6? | Thiếu lazy propagation, thiếu cập nhật đoạn, chưa đóng gói thành lớp, chưa tách kiểu `Value`/`Tag`. |
| 3 | Thiết kế lớp `SegmentTree` thế nào cho đúng yêu cầu "tách rõ loại dữ liệu của giá trị cây và giá trị cập nhật"? | Đặt hai alias `Value` và `Tag` với chú thích ngữ nghĩa; `apply` nhân thẻ với độ dài đoạn. |
| 4 | Làm sao bảo đảm đáp án chuẩn của test cố định là đúng? | Chạy chéo hai cài đặt độc lập, chỉ ghi `.out` khi trùng nhau; ngoài ra tính tay 8 test nhỏ. |
| 5 | Cách đo thời gian nào tránh được nhiễu và tránh bị trình tối ưu loại bỏ vòng lặp? | Sinh trước chuỗi thao tác, chạy lặp lấy giá trị nhỏ nhất, dùng `checksum` để giữ vòng lặp. |
| 6 | Điều kiện nào bảo đảm không tràn 64-bit? | `N · (A + T) < 2^63`; bổ sung kiểm tra khi chạy bằng cờ `-DST_CHECK_OVERFLOW`. |

---

## 4. Phần đã được kiểm chứng độc lập

Mọi khẳng định dưới đây đều đã được **chạy thật và kiểm tra**, không chấp nhận
theo lời công cụ.

### 4.1. Lỗi trong bản nháp — đã kiểm chứng bằng phản ví dụ cụ thể

Công cụ chỉ ra rằng `build` dùng `t[v] = t[2v] + t[2v+1]` (tổng) trong khi
`update` và `query` dùng `min`. Nhóm **kiểm chứng lại bằng tay**: với
`a = [1, 2, 8, 9, 3]`, truy vấn min trên đoạn `[3, 4]` (1-based) rơi đúng vào
một nút trong có giá trị `8 + 9 = 17`, nên bản nháp trả `17` thay vì `8`.

Phản ví dụ này đã được biến thành **test hồi quy** trong nhóm test đối kháng
(`tests/unit_tests.cpp`, mục "hoi quy").

### 4.2. Kết quả 8 test cố định — đã tính tay và đối chiếu ba nguồn

| Test | Tính tay | Cây phân đoạn | Mảng thường | Khớp |
|---|---|---|---|---|
| `01_basic_sample` | 23, 19, 38, 34, 13 | như cột trái | như cột trái | ✔ |
| `02_single_element` | 42, −58, 999999942 | ✔ | ✔ | ✔ |
| `03_full_range` | 8, 88, −72, −18 | ✔ | ✔ | ✔ |
| `04_point_ops` | 21, 3, 14 | ✔ | ✔ | ✔ |
| `05_negative_values` | −15, −65, 173, 135 | ✔ | ✔ | ✔ |
| `06_non_power_of_two` | 91, 791, 728, 63, 145, 591 | ✔ | ✔ | ✔ |
| `07_zero_delta` | 10, 10, 5, 1, 4 | ✔ | ✔ | ✔ |
| `08_large_values_64bit` | 5000000000, 20000000000, 4000000000 | ✔ | ✔ | ✔ |

### 4.3. Đối chiếu ngẫu nhiên quy mô lớn

3000 vòng × 500 thao tác ≈ **1,5 triệu thao tác ngẫu nhiên** đối chiếu với lời
giải mảng thường, không có sai lệch. Kết quả lưu tại `results/test_log.txt`.

### 4.4. Biên dịch sạch với cảnh báo nghiêm ngặt

Toàn bộ mã nguồn dịch **không phát sinh cảnh báo nào** với
`-Wall -Wextra -Wshadow -Wconversion -pedantic`.

### 4.5. Số liệu thực nghiệm — đo trên máy thật, không lấy từ công cụ

Toàn bộ số liệu trong `results/` được sinh bằng cách **chạy thật** trên máy của
nhóm; `results/environment.md` ghi lại cấu hình máy và cờ dịch. Không có con số
nào trong báo cáo được lấy từ trí nhớ của công cụ.

Một dự đoán lý thuyết đã được thực nghiệm xác nhận: cấu trúc chiếm `4N + 4N` ô
64-bit, tức **đúng 64 byte cho mỗi phần tử** — số đo thực tế cho `64,0` byte ở
cả bốn mốc `N`.

---

## 5. Phần nhóm tự phát hiện và tự sửa

### 5.1. Hằng số mong đợi sai trong một test do công cụ sinh ra

Công cụ viết test `cong gia tri duong len doan am` với giá trị mong đợi `171`.
Khi chạy, test **hỏng**. Nhóm kiểm tra lại bằng tay:

```
a = [−5, −4, −3, −2, −1]
cộng −10 vào toàn đoạn  →  [−15, −14, −13, −12, −11]
cộng 100 vào [1, 2]     →  a[1] = 86, a[2] = 87
tổng [1, 2] = 86 + 87   =  173      ← không phải 171
```

Kết luận: **cây phân đoạn đúng, hằng số trong test sai**. Đã sửa giá trị mong
đợi thành `173` và ghi chú cách tính ngay trong mã. Đây là minh chứng cho việc
kết quả do công cụ sinh ra **phải được kiểm chứng lại**, kể cả phần kiểm thử.

### 5.2. Lỗi tự gây ra trong dữ liệu test cố định

Tệp `03_full_range.in` ban đầu khai `Q = 7` nhưng chỉ có 6 dòng thao tác. Bộ đọc
dữ liệu phát hiện và báo `Loi dinh dang du lieu vao: thieu loai truy van`. Đã
sửa thành `Q = 6`. Sự cố này cho thấy giá trị của việc **kiểm tra dữ liệu vào**
ngay trong chương trình thay vì tin tưởng tệp đầu vào.

### 5.3. Sự cố môi trường: xung đột DLL trên Windows

Sau khi biên dịch, mọi tệp `.exe` đổ vỡ với mã `0xC0000005` ngay khi tạo một đối
tượng `std::ofstream` — nhưng **chỉ khi chạy từ PowerShell**, còn chạy từ shell
của MSYS2 thì bình thường.

Quá trình truy vết (đã ghi lại đầy đủ vì đây là một bài học kỹ thuật đáng giá):

1. Ban đầu nghi là lỗi hành vi không xác định (undefined behavior) vì bản `-O0`
   chạy được còn bản `-O2` thì đổ vỡ.
2. Không có sẵn AddressSanitizer/UBSan trên MinGW, cũng không có `gdb`, nên
   chuyển sang **thu hẹp bằng chương trình tối giản**.
3. Rút gọn dần xuống còn đúng hai dòng — `std::ofstream fout;` cộng một lệnh in
   ra `std::cout` — vẫn đổ vỡ. Điều đó loại trừ hoàn toàn khả năng lỗi nằm trong
   thuật toán.
4. Kiểm tra `PATH` và phát hiện `C:\Program Files\Git\mingw64\bin\libstdc++-6.dll`
   (do Git for Windows cài) được nạp thay cho DLL của MSYS2 → **xung đột ABI**.
5. Khắc phục bằng **liên kết tĩnh** (`-static -static-libgcc -static-libstdc++`),
   nhờ đó các tệp `.exe` chạy độc lập trên máy chưa cài MSYS2.

Nguyên nhân này **không** liên quan gì đến thuật toán, nhưng nếu không truy ra
thì rất dễ đổ oan cho mã nguồn. Đã ghi vào mục "Khắc phục sự cố" của `README.md`.

### 5.4. **[NHÓM TỰ VIẾT]** Các phần nhóm tự viết hoàn toàn

Liệt kê ở đây những phần nhóm tự viết không có hỗ trợ của công cụ, ví dụ:

* Chứng minh bảo toàn bất biến (I1), (I2) trong mục 3.6 của báo cáo.
* Chứng minh số nút chạm tới là `O(log N)` trong mục 3.3.
* Phần so sánh với Fenwick tree ở mục 1.6.
* Phần ứng dụng minh họa và phân tích giới hạn ở mục 7.
* Phần kết luận ở mục 9.
* Slide và bản demo.

---

## 6. Đánh giá về việc dùng công cụ hỗ trợ

**[NHÓM TỰ VIẾT]** Gợi ý các ý cần nêu:

* Công cụ hữu ích nhất ở khâu nào (rà soát lỗi, sinh khung kiểm thử, viết script)?
* Công cụ mắc lỗi gì và nhóm phát hiện ra bằng cách nào? (xem mục 5.1)
* Vì sao việc **kiểm chứng độc lập** là bắt buộc, không được tin ngay kết quả?
* Bài học rút ra cho những lần sau.
