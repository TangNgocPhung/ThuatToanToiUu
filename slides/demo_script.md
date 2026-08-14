# Kịch bản thuyết trình quay demo (10–15 phút) — bản chi tiết

Chuyên đề 6 — Cây phân đoạn (Segment Tree) và Lazy Propagation
Người trình bày: Tăng Ngọc Phụng (phụ trách quay demo + code theo bảng phân công)

Tổng thời lượng mục tiêu: **~13–14 phút** (nằm trong khung 10–15 phút, còn dư ~1 phút phòng khi nói chậm).
Mọi câu thoại, mọi lệnh, mọi số liệu dưới đây đã đối chiếu với `README.md`, mã nguồn thật, và `results/results.md` — **không cần tự tra lại**, cứ đọc và gõ theo.

---

## Bảng mốc thời gian tổng quan (dán lên màn hình phụ để canh giờ)

| Mốc | Nội dung | Thời lượng |
|---|---|---|
| 0:00 – 1:00 | Giới thiệu | 1:00 |
| 1:00 – 1:40 | Bài toán & vì sao cần Segment Tree | 0:40 |
| 1:40 – 2:05 | Kiến trúc file / thư mục | 0:25 |
| 2:05 – 2:45 | `Value` / `Tag` tách bạch | 0:40 |
| 2:45 – 3:35 | `build()` + vì sao cấp phát 4N | 0:50 |
| 3:35 – 4:05 | `update_range` / `query_range` — API tổng quan | 0:30 |
| 4:05 – 5:25 | Hai bất biến (I1), (I2) | 1:20 |
| 5:25 – 6:15 | `apply` — ví dụ số cụ thể | 0:50 |
| 6:15 – 6:55 | `push_down` | 0:40 |
| 6:55 – 8:15 | `update_rec` — ba trường hợp | 1:20 |
| 8:15 – 8:45 | `query_rec` — so sánh nhanh | 0:30 |
| 8:45 – 9:25 | Vì sao O(log N) — bổ đề chặn số nút mỗi mức | 0:40 |
| 9:25 – 9:55 | `main.cpp` — FastScanner, chọn engine, `NaiveArray` | 0:30 |
| 9:55 – 10:35 | Demo: ví dụ cơ bản tính tay | 0:40 |
| 10:35 – 11:05 | Demo: đối chiếu `--engine=naive` | 0:30 |
| 11:05 – 12:05 | Demo: `run_tests.ps1` — toàn bộ pipeline kiểm thử | 1:00 |
| 12:05 – 13:05 | Demo: `bench.exe` + số liệu `results/results.md` | 1:00 |
| 13:05 – 13:35 | Mở rộng đã cài đặt (gán/cộng, affine) | 0:30 |
| 13:35 – 14:20 | Kết luận | 0:45 |

Nếu thiếu giờ, hai chỗ **an toàn để cắt gọn** là "Mở rộng đã cài đặt" (13:05–13:35) và phần đọc số liệu chi tiết trong `results/results.md` (rút còn 1 câu tóm tắt).

---

## PHẦN 0 — Chuẩn bị trước khi bấm quay (không tính vào 10–15 phút)

1. Mở PowerShell tại thư mục gốc dự án, kiểm tra `bin/` đã có sẵn `.exe` — **không build lại khi đang quay**, tốn thời gian và dễ dính lỗi biên dịch trực tiếp trên video:
   ```powershell
   ls bin
   ```
2. Mở VS Code, mở sẵn 3 tab theo đúng thứ tự sẽ giảng: `src/segment_tree.hpp`, `src/main.cpp`, `src/segment_tree_ext.hpp`. Cuộn `segment_tree.hpp` sẵn tới dòng 51 (đầu lớp) để lúc quay không mất thời gian tìm — sau đó cuộn tiếp theo mốc dòng ghi ở từng mục bên dưới.
3. Tăng cỡ chữ terminal + VS Code (`Ctrl` + cuộn chuột hoặc `Ctrl -/+`) — người xem trên điện thoại/màn hình nhỏ vẫn phải đọc được.
4. Mở sẵn `results/results.md` ở một tab trình duyệt hoặc VS Code, cuộn tới Bảng 4.4/4.5 (số liệu ns/op theo n) để chuyển qua nhanh ở phút 12.
5. **Chạy thử toàn bộ kịch bản một lượt, không quay**, để chắc chắn không lệnh nào lỗi giữa chừng.
6. Đóng cửa sổ/tab không liên quan, tắt thông báo (Focus Assist / Do Not Disturb).
7. Chuẩn bị đồng hồ bấm giờ hoặc app quay có hiển thị thời gian, để tự canh theo bảng mốc ở trên.

---

## PHẦN 1 — Giới thiệu (0:00 – 1:00)

*Camera/màn hình: có thể để slide tiêu đề hoặc màn hình VS Code tĩnh.*

**Nói:**

> "Xin chào thầy và các bạn. Em là Phụng, đại diện nhóm 3 trình bày phần demo cho chuyên đề Cây phân đoạn — Segment Tree — kết hợp Lazy Propagation, môn Các thuật toán tối ưu, giảng viên hướng dẫn thầy Nguyễn Duy Hàm.
>
> Trong khoảng 13 phút tới, em sẽ đi qua ba phần chính: thứ nhất, giải thích chi tiết phần code cài đặt thuật toán; thứ hai, chạy demo trực tiếp chương trình cùng toàn bộ bộ kiểm thử tự động; và thứ ba, xem qua số liệu thực nghiệm để đối chiếu với lý thuyết."

---

## PHẦN 2 — Bài toán & vì sao cần Segment Tree (1:00 – 1:40)

**Nói:**

> "Bài toán nhóm giải quyết: cho một dãy N số nguyên, cần xử lý xen kẽ Q thao tác gồm hai loại — thao tác một, cộng một giá trị V vào MỌI phần tử trong đoạn [L, R]; thao tác hai, truy vấn tổng các phần tử trong đoạn [L, R]. Ràng buộc thực tế: N và Q có thể lên tới 10 mũ 5, 10 mũ 6.
>
> Nếu làm trực tiếp bằng mảng thường — mỗi lần cập nhật đoạn thì lặp qua từng phần tử, mỗi lần truy vấn thì cộng dồn từng phần tử — mỗi thao tác tốn O(N), tổng cộng O(N nhân Q). Với N, Q cỡ triệu thì con số này lên tới hàng nghìn tỷ phép tính, không thể chạy kịp trong thời gian giới hạn.
>
> Cây phân đoạn kết hợp Lazy Propagation giải quyết vấn đề này bằng cách đưa CẢ HAI thao tác — cập nhật đoạn lẫn truy vấn đoạn — về cùng độ phức tạp O(log N). Đây chính là nội dung code em sắp trình bày."

---

## PHẦN 3 — Kiến trúc file / thư mục (1:40 – 2:05)

Chuyển sang VS Code, cho xem nhanh sidebar thư mục `src/`.

**Nói:**

> "Toàn bộ thuật toán nằm trong `src/segment_tree.hpp` — một file header-only, không phụ thuộc thư viện ngoài, có thể copy sang dự án khác dùng ngay. Bên cạnh đó, `naive.hpp` chứa lời giải mảng thường dùng làm bộ tham chiếu, `main.cpp` là chương trình đọc dữ liệu và gọi thuật toán, `bench.cpp` dùng để đo hiệu năng, và `segment_tree_ext.hpp` chứa hai biến thể mở rộng em sẽ giới thiệu nhanh ở cuối."

---

## PHẦN 4 — Giải thích code chi tiết (2:05 – 9:55)

Mở tab `src/segment_tree.hpp`.

### 4.1. `Value` và `Tag` tách bạch (2:05 – 2:45)

Cuộn tới dòng **51–59**.

**Nói:**

> "Lớp chính tên là `SegmentTree`. Điểm đầu tiên đáng chú ý — đúng theo yêu cầu của đề: hai khái niệm `Value` và `Tag` được tách bạch rõ ràng dù cùng là `long long` 64-bit.
>
> `Value` là kiểu của giá trị TỔNG HỢP lưu tại một nút — tức tổng của cả đoạn mà nút đó quản lý.
>
> `Tag` là kiểu của giá trị CẬP NHẬT đang hoãn lại — tức lượng cộng cho MỘT phần tử, chưa được đẩy xuống con.
>
> Vì sao phải tách tên, dù bản chất cùng là số nguyên 64-bit? Vì nhầm giữa 'tổng của cả đoạn' và 'lượng cộng cho một phần tử' chính là lỗi phổ biến nhất khi cài lazy propagation — đặt tên kiểu riêng buộc người đọc code phải phân biệt rõ ngay từ đầu."

### 4.2. `build()` và vì sao cấp phát 4N (2:45 – 3:35)

Cuộn tới dòng **75–80**.

**Nói:**

> "`build(a)` dựng cây từ mảng ban đầu, độ phức tạp O(N) — vì hàm đệ quy đi qua mỗi nút đúng một lần.
>
> Điểm kỹ thuật đáng chú ý: mảng `tree_` và `lazy_` được cấp phát 4 nhân N ô nhớ, không phải 2N. Lý do: cây phân đoạn đệ quy trên đoạn [0, N-1] KHÔNG phải lúc nào cũng là cây nhị phân hoàn chỉnh — chỉ hoàn chỉnh khi N là lũy thừa của 2. Khi N lẻ hoặc không tròn lũy thừa 2, chiều cao cây là h xấp xỉ log cơ số 2 của N làm tròn lên, và chỉ số nút lớn nhất có thể chạm tới nhỏ hơn 2 mũ (h cộng 1), mà đại lượng này luôn nhỏ hơn hoặc bằng 4N. Vậy 4N là cận an toàn dùng chung cho MỌI giá trị N, không cần biết trước N có phải lũy thừa của 2 hay không."

### 4.3. `update_range` / `query_range` — API tổng quan (3:35 – 4:05)

Cuộn tới dòng **88–100**.

**Nói:**

> "Đây là hai hàm public mà `main.cpp` sẽ gọi trực tiếp. `update_range(l, r, v)` cộng v vào mọi phần tử trong đoạn [l, r], độ phức tạp O(log N). `query_range(l, r)` trả về tổng đoạn [l, r], cũng O(log N). Chỉ số dùng quy ước 0-based, đoạn đóng — tầng đọc dữ liệu ở `main.cpp` chịu trách nhiệm đổi từ 1-based sang 0-based trước khi gọi vào đây."

### 4.4. Hai bất biến — trái tim của Lazy Propagation (4:05 – 5:25)

Cuộn tới khối comment "BẤT BIẾN CỦA CÂY", dòng **133–144**.

**Nói:**

> "Đây là phần quan trọng nhất của toàn bộ thuật toán — hai bất biến mà mọi thao tác phải giữ đúng xuyên suốt.
>
> Bất biến một, ký hiệu I1: `tree_[v]` luôn là giá trị tổng ĐÚNG của đoạn mà nút v quản lý — đã tính cả phần cập nhật đang treo tại `lazy_[v]` của chính nút đó.
>
> Bất biến hai, ký hiệu I2: `lazy_[v]` là cập nhật ĐÃ được áp vào `tree_[v]` rồi, nhưng CHƯA được đẩy xuống hai con của v.
>
> Hệ quả trực tiếp, rất quan trọng: đọc `tree_[v]` ở BẤT KỲ thời điểm nào cũng cho kết quả đúng ngay lập tức — không cần đẩy thẻ trước khi đọc. Chỉ khi nào thuật toán cần ĐI XUỐNG hai con — tức khi đoạn của v giao một phần với đoạn đang xử lý, chưa nằm trọn cũng chưa nằm ngoài — lúc đó mới bắt buộc gọi `push_down` để khôi phục bất biến cho hai con trước khi đệ quy tiếp. Toàn bộ tính đúng đắn của thuật toán, nhóm chứng minh bằng quy nạp dựa trên đúng hai bất biến này, trình bày chi tiết ở Chương 3 của báo cáo kỹ thuật."

### 4.5. `apply` — ví dụ số cụ thể (5:25 – 6:15)

Cuộn tới dòng **152–169**.

**Nói:**

> "`apply(v, len, t)` áp một thẻ cập nhật t lên nút v đại diện đoạn dài `len` phần tử. Hai việc xảy ra: cộng `t nhân len` vào `tree_[v]`, và cộng dồn t vào `lazy_[v]`.
>
> Ví dụ cụ thể cho dễ hình dung: nếu một nút quản lý đoạn dài 4 phần tử, đang có tổng là 20, và ta áp thẻ cộng 3 cho mỗi phần tử, thì tổng mới phải là 20 cộng 3 nhân 4, tức 32 — đúng bằng dòng code `tree_[v] += t * len`. Còn `lazy_[v]` chỉ đơn giản cộng dồn t, vì phép cộng vừa giao hoán vừa kết hợp nên nhiều lần hoãn gộp lại được thành một con số duy nhất, không cần lưu danh sách các lần cập nhật riêng lẻ."

### 4.6. `push_down` (6:15 – 6:55)

Cuộn tới dòng **179–184**.

**Nói:**

> "`push_down` chỉ thực sự làm việc khi `lazy_[v]` khác 0 — nếu không có gì đang treo thì thoát ngay, không tốn công. Khi có, hàm gọi `apply` cho cả hai con, mỗi con với đúng độ dài đoạn của riêng nó — con trái dài `tm - tl + 1` phần tử, con phải dài `tr - tm` phần tử — rồi đặt `lazy_[v]` về 0 vì đã đẩy xong, không còn gì treo tại v nữa."

### 4.7. `update_rec` — ba trường hợp kinh điển (6:55 – 8:15)

Cuộn tới dòng **207–218**.

**Nói:**

> "Đây là nơi thể hiện rõ nhất lợi ích thật sự của lazy propagation. Hàm `update_rec` xét quan hệ giữa đoạn `[tl, tr]` mà nút hiện tại quản lý, và đoạn `[l, r]` cần cập nhật — có đúng ba trường hợp.
>
> Trường hợp một: hai đoạn không giao nhau — điều kiện `r < tl` hoặc `tr < l` — dừng ngay, không làm gì cả.
>
> Trường hợp hai, quan trọng nhất: đoạn của nút nằm TRỌN trong đoạn cần cập nhật. Lúc này chỉ gọi `apply` đúng một lần rồi DỪNG LUÔN — không đi sâu xuống cây con nữa, dù cây con có thể còn hàng nghìn nút bên dưới. Đây chính xác là chỗ lazy propagation tiết kiệm thời gian so với việc lặp cập nhật từng phần tử một.
>
> Trường hợp ba: hai đoạn giao nhau một phần — không nằm trọn, cũng không nằm ngoài. Lúc này bắt buộc gọi `push_down` trước để đẩy thẻ cũ xuống, giữ đúng bất biến cho con, rồi mới đệ quy xuống cả hai con, và cuối cùng gọi `pull` để tính lại giá trị nút cha từ hai con vừa cập nhật xong."

### 4.8. `query_rec` — so sánh nhanh (8:15 – 8:45)

Cuộn tới dòng **220–227**.

**Nói:**

> "`query_rec` đi theo đúng cấu trúc ba trường hợp y hệt `update_rec` — chỉ khác ở chỗ trường hợp hai không gọi `apply` mà trả về ngay `tree_[v]` vì bất biến I1 đảm bảo giá trị đó đã đúng; và trường hợp ba, sau khi `push_down`, kết quả là gộp hai giá trị trả về từ hai con lại với nhau bằng phép cộng."

### 4.9. Vì sao O(log N) — trực giác của bổ đề chặn số nút (8:45 – 9:25)

*(Không cần cuộn code, chỉ nói — có thể vẽ tay nhanh trên giấy hoặc dùng con trỏ chỉ vào cây ở slide nếu có sẵn.)*

**Nói:**

> "Vì sao cả update lẫn query đều chỉ tốn O(log N)? Trực giác là: ở mỗi mức của cây, chỉ có nhiều nhất một vài nút — chặn trên là một hằng số, cụ thể là bốn — rơi vào trường hợp ba, tức bị 'chia tách' và phải đệ quy tiếp xuống cả hai con. Còn lại, mọi nút khác ở mức đó đều dừng ngay ở trường hợp một hoặc trường hợp hai. Vì cây có chiều cao O(log N), và mỗi mức chỉ tốn công hằng số, nên tổng số nút được thăm qua toàn bộ quá trình đệ quy là O(log N). Chứng minh đầy đủ bằng quy nạp, nhóm trình bày chi tiết ở Chương 3, mục 3.5 của báo cáo kỹ thuật."

### 4.10. `main.cpp` — FastScanner, chọn engine, NaiveArray (9:25 – 9:55)

Chuyển tab sang `src/main.cpp`, cuộn tới dòng **215–216** (dispatch theo engine).

**Nói:**

> "`main.cpp` dùng một bộ đọc nhanh tên `FastScanner` để nạp toàn bộ dữ liệu vào bộ nhớ một lần, cần thiết vì thực nghiệm chạy tới hàng triệu thao tác. Chương trình cho phép chọn một trong hai engine bằng tham số dòng lệnh: mặc định là `SegmentTree`, hoặc `--engine=naive` để chạy `NaiveArray` — lời giải mảng thường, cài đơn giản, dùng làm bộ tham chiếu để đối chiếu kết quả. Bây giờ em chuyển qua terminal để chạy thực tế."

---

## PHẦN 5 — Demo chạy thực tế (9:55 – 13:05)

Chuyển sang cửa sổ PowerShell, đứng tại thư mục gốc dự án.

### 5.1. Ví dụ cơ bản, tính tay được (9:55 – 10:35)

**Nói:**

> "Trước tiên em chạy một ví dụ nhỏ, tính tay được, để mọi người thấy chương trình hoạt động đúng ngay từ đầu."

Gõ:

```powershell
cat tests\cases\01_basic_sample.in
```

**Nói:**

> "Đây là mảng 5 phần tử [1, 2, 8, 9, 3], với 6 thao tác: hỏi tổng cả mảng, hỏi tổng đoạn [2,4], cộng 5 vào đoạn [2,4], rồi hỏi lại ba lần nữa ở các đoạn khác nhau."

Gõ:

```powershell
.\bin\segtree.exe tests\cases\01_basic_sample.in
```

**Nói:**

> "Kết quả in ra lần lượt là 23, 19, 38, 34, 13 — khớp chính xác đáp án tính tay: tổng ban đầu 1 cộng 2 cộng 8 cộng 9 cộng 3 bằng 23; đoạn [2,4] là 2 cộng 8 cộng 9 bằng 19. Sau khi cộng 5 vào đoạn [2,4], mảng thành 1, 7, 13, 14, 3 — tổng cả mảng lúc này là 38, đoạn [2,4] là 34, còn riêng phần tử thứ 3 là 13."

### 5.2. Đối chiếu với lời giải mảng thường (10:35 – 11:05)

**Nói:**

> "Nhóm luôn đối chiếu Segment Tree với lời giải mảng thường độc lập, đóng vai trò bộ tham chiếu, để tự tin là kết quả đúng chứ không chỉ 'nhìn có vẻ đúng'."

Gõ:

```powershell
.\bin\segtree.exe --engine=naive tests\cases\01_basic_sample.in
```

**Nói:**

> "Kết quả giống hệt bản Segment Tree vừa chạy — hai cài đặt hoàn toàn độc lập về thuật toán cho ra cùng một đáp án."

### 5.3. Chạy toàn bộ pipeline kiểm thử (11:05 – 12:05)

**Nói:**

> "Tiếp theo em chạy toàn bộ pipeline kiểm thử tự động của nhóm."

Gõ:

```powershell
.\scripts\run_tests.ps1
```

**Nói (khi script đang chạy):**

> "Script này thực hiện bốn bước tuần tự. Bước một, kiểm thử đơn vị với 71 khẳng định, chia làm năm nhóm: cơ bản, biên, đối kháng, ngẫu nhiên đối chiếu, và mở rộng. Bước hai, chạy lại toàn bộ với cờ kiểm tra tràn số 64-bit được bật. Bước ba, chạy 12 test cố định trong thư mục `tests/cases`, so từng dòng kết quả với đáp án chuẩn — đáp án chuẩn này được xác thực chéo bằng cả hai cài đặt, chỉ ghi nhận khi hai bên trùng nhau, và tám test đầu còn được tính tay độc lập. Bước bốn, stress test hàng nghìn kịch bản dữ liệu ngẫu nhiên, đối chiếu với mảng thường sau mỗi bước lẫn ở trạng thái cuối."

Khi script chạy xong, chỉ con trỏ vào dòng kết quả cuối.

**Nói:**

> "Toàn bộ đều PASS, không phát hiện sai lệch nào so với bộ tham chiếu. Log đầy đủ được ghi vào `results/test_log.txt`."

### 5.4. Đo hiệu năng và đối chiếu số liệu (12:05 – 13:05)

**Nói:**

> "Cuối cùng, em chạy nhanh một phép đo hiệu năng để minh họa trực quan độ phức tạp O(log N)."

Gõ:

```powershell
.\bin\bench.exe --sizes=1000,10000,100000 --ops=50000 --repeat=3
```

**Nói:**

> "Cột `ns/op` là thời gian trung bình mỗi thao tác, tính bằng nano giây. Có thể thấy khi N tăng gấp 10 lần, gấp 100 lần, thời gian mỗi thao tác chỉ tăng nhẹ chứ không tăng theo tỉ lệ với N."

Chuyển qua tab đã mở sẵn `results/results.md`, chỉ vào Bảng 4.4 và 4.5.

**Nói:**

> "Đây là số liệu đầy đủ nhóm đo được trên N tới một triệu phần tử. Với thao tác truy vấn, thời gian mỗi lần chỉ tăng từ 218,6 nano giây ở N bằng một nghìn, lên 1024,6 nano giây ở N bằng một triệu — dù N đã tăng một nghìn lần. So với duyệt trực tiếp: ở N bằng mười nghìn, Segment Tree nhanh hơn khoảng 6 lần; ở N bằng một trăm nghìn, nhanh hơn khoảng 40 lần, vì duyệt trực tiếp tăng gần như tuyến tính theo N còn Segment Tree gần như phẳng. Số liệu này chính là căn cứ cho các biểu đồ trong báo cáo kỹ thuật của nhóm."

---

## PHẦN 6 — Mở rộng đã cài đặt (13:05 – 13:35)

Chuyển tab sang `src/segment_tree_ext.hpp`, cuộn tới dòng đầu file (comment mô tả, dòng 1–12).

**Nói:**

> "Ngoài lớp chính, nhóm còn cài thêm hai biến thể trong file `segment_tree_ext.hpp`, để chứng minh khung apply / push_down / pull vừa trình bày là TỔNG QUÁT — chỉ cần đổi cặp `Value`/`Tag` và luật hợp thành thẻ là chuyển sang bài toán khác.
>
> Biến thể một, `SegmentTreeAssignAdd`: hỗ trợ cả gán đoạn lẫn cộng đoạn, truy vấn được tổng, min, max cùng lúc — thẻ hợp thành không giao hoán, vì một phép gán mới sẽ xóa sạch mọi phép cộng đứng trước nó.
>
> Biến thể hai, `SegmentTreeAffine`: cập nhật dạng affine x tới a nhân x cộng b theo modulo p — ví dụ chuẩn mực cho việc hợp thành hai thẻ không giao hoán, vì thứ tự áp dụng ảnh hưởng tới kết quả cuối.
>
> Cả hai đều được kiểm thử đối chiếu với lời giải mảng thường ngay trong nhóm test 'mở rộng' của `unit_tests.exe` mà em vừa chạy ở bước trước — đều PASS."

---

## PHẦN 7 — Kết luận (13:35 – 14:20)

**Nói:**

> "Tóm lại, nhóm đã cài đặt Segment Tree kết hợp Lazy Propagation, giải đúng bài toán cập nhật đoạn – truy vấn tổng đoạn với độ phức tạp O(log N) mỗi thao tác, O(N) bộ nhớ. Tính đúng đắn được chứng minh bằng quy nạp qua hai bất biến giá trị và thẻ trì hoãn, rồi được xác nhận lại bằng kiểm thử đối chiếu với lời giải mảng thường qua hàng nghìn kịch bản ngẫu nhiên, không phát hiện sai lệch nào. Thực nghiệm cho thấy chi phí mỗi thao tác gần như phẳng khi N tăng, đúng như dự đoán O(log N), và nhanh hơn duyệt trực tiếp hàng chục lần khi N đủ lớn. Khung apply / push_down / pull cũng được chứng minh là tổng quát, áp dụng được cho các phép gộp và cập nhật khác thông qua hai biến thể mở rộng.
>
> Em xin cảm ơn thầy và các bạn đã theo dõi, rất mong nhận được câu hỏi và góp ý."

---

## PHẦN 8 — Dự phòng hỏi–đáp (chuẩn bị tinh thần, không cần quay)

Nếu thầy hỏi trực tiếp sau khi xem video hoặc trong buổi bảo vệ, đây là vài câu hỏi khả năng cao sẽ gặp và hướng trả lời bám sát mã nguồn/báo cáo thật:

**Q: Vì sao không dùng Binary Indexed Tree (Fenwick) cho gọn hơn?**
> BIT gọn và nhanh hơn về hằng số, nhưng chủ yếu hợp với các phép gộp KHẢ NGHỊCH (như tổng, dùng kỹ thuật cộng/trừ hai điểm), khó mở rộng tự nhiên cho min/max hay cập nhật đoạn phức tạp. Segment Tree chỉ cần tính KẾT HỢP, không cần khả nghịch, nên tổng quát hơn — đúng như Bảng 4.3 trong báo cáo.

**Q: Nếu phép gộp đổi từ tổng sang max thì cập nhật đoạn có còn dùng lazy được không?**
> Được, miễn cập nhật hợp thành được thành thẻ kích thước hằng số. Ví dụ "gán toàn đoạn" hợp với max dễ dàng — nhóm đã cài trong `SegmentTreeAssignAdd`. Nhưng "cộng thêm rồi lấy max" cũng vẫn dùng lazy tag cộng dồn được bình thường vì effect(val, len) không phụ thuộc thứ tự phần tử.

**Q: Vì sao dùng `long long` mà không phải `int`?**
> Vì tổng một đoạn có thể vượt giới hạn 32-bit khi N và giá trị phần tử đều lớn — README mục 1 nêu rõ điều kiện an toàn: N nhân (A cộng T) phải nhỏ hơn 2 mũ 63, với A là trị tuyệt đối lớn nhất của phần tử, T là tổng trị tuyệt đối các lượng cập nhật. Bản build có cờ `-DST_CHECK_OVERFLOW` sẽ tự kiểm tra và dừng chương trình nếu vi phạm.

**Q: N = 0 hoặc N = 1 thì xử lý ra sao?**
> Đã có test biên riêng cho hai trường hợp này (`Boundary Test`, xem Bảng 6.2 báo cáo). Với N = 0, `build` cấp phát tối thiểu 2 ô để tránh mảng rỗng gây lỗi chỉ số. Với N = 1, cây chỉ có một nút lá, mọi truy vấn/cập nhật đều rơi ngay vào trường hợp "nằm trọn".

**Q: Bộ nhớ có thể giảm xuống 2N thay vì 8N (tree + lazy) không?**
> Có kỹ thuật cây phân đoạn "không đệ quy" (iterative) dùng đúng 2N, nhưng đánh đổi là cài lazy propagation phức tạp hơn nhiều vì mất đi cấu trúc đệ quy tự nhiên để push-down đúng thứ tự. Nhóm chọn bản đệ quy 4N vì rõ ràng, dễ chứng minh đúng, và 8N vẫn chỉ là O(N) — không đổi bậc tiệm cận.

---

## PHẦN 9 — Mẹo kỹ thuật khi quay/dựng video

* Dùng OBS Studio (miễn phí) hoặc Xbox Game Bar (`Win + G`, có sẵn trên Windows) để quay màn hình + tiếng.
* Quay riêng từng phần (code, terminal) rồi ghép lại trong bước dựng sẽ dễ sửa hơn quay liền một mạch — nếu lỡ vấp câu ở phút 8, chỉ cần quay lại đúng đoạn đó.
* Đoạn `run_tests.ps1` chạy có thể mất 30 giây đến vài phút tùy máy — nếu lâu, quay xong rồi TUA NHANH (speed-up 3–4x) đoạn chờ khi dựng, giữ nguyên lời thoại giải thích đọc trước/sau.
* Xuất video độ phân giải tối thiểu 1080p, chữ trong terminal/VS Code phải đọc rõ khi xem trên điện thoại.
* Kiểm tra lại tổng thời lượng nằm trong khung 10–15 phút TRƯỚC khi upload.

---

## Bảng tóm tắt lệnh dùng khi quay (copy nhanh)

```powershell
ls bin
cat tests\cases\01_basic_sample.in
.\bin\segtree.exe tests\cases\01_basic_sample.in
.\bin\segtree.exe --engine=naive tests\cases\01_basic_sample.in
.\scripts\run_tests.ps1
.\bin\bench.exe --sizes=1000,10000,100000 --ops=50000 --repeat=3
```

## Mốc dòng code cần cuộn tới

`src/segment_tree.hpp`: 51–59 (Value/Tag) → 75–80 (build, 4N) → 88–100 (update_range/query_range) → 133–144 (hai bất biến) → 152–169 (apply) → 179–184 (push_down) → 207–218 (update_rec) → 220–227 (query_rec).

`src/main.cpp`: 1–22 (cách dùng) → 215–216 (chọn engine SegmentTree/NaiveArray).

`src/segment_tree_ext.hpp`: 1–12 (comment tổng quan hai biến thể).
