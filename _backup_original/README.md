# Bản nháp gốc (lưu lại để đối chiếu)

Thư mục này giữ nguyên bản cài đặt đầu tiên của nhóm, **trước khi** tái cấu trúc
dự án. Mục đích duy nhất là làm bằng chứng cho mục 5.3 của báo cáo
(`report/report.md`) — nơi phân tích lỗi đã phát hiện.

| Tệp | Nội dung |
|---|---|
| `main_original.cpp` | bản cài đặt nháp |
| `generator_original.cpp` | bộ sinh dữ liệu nháp |
| `input_original.txt`, `output_original.txt` | dữ liệu thử nghiệm nháp |

## Tóm tắt các vấn đề của bản nháp

**Lỗi sai kết quả**

1. `build` gộp bằng **tổng** (`t[v] = t[2*v] + t[2*v+1]`) nhưng `update` và
   `query` gộp bằng **min** — cây lai hai phép gộp nên trả sai. Với
   `a = [1, 2, 8, 9, 3]`, truy vấn min trên `[3, 4]` (1-based) trả `17` thay vì `8`.
2. `query` dùng `LLONG_MAX` làm phần tử trung hòa của min; nếu chuyển sang phép
   tổng thì tràn ngay.

**Thiếu so với yêu cầu của chuyên đề 6**

3. Không có lazy propagation — trọng tâm của đề.
4. Chỉ có cập nhật **một điểm**; đề yêu cầu cập nhật **đoạn**.
5. Không đóng gói thành lớp `SegmentTree` với `build` / `update_range` /
   `query_range` / `apply` / `push_down`.
6. Không tách kiểu dữ liệu của giá trị cây và giá trị cập nhật.

**Vấn đề kỹ thuật**

7. Dùng `LLONG_MAX` mà không `#include <climits>`.
8. `generator.cpp` sinh sai định dạng (in `l r`, còn `main` đọc 3 số mỗi truy
   vấn) nên không thể đối chiếu tự động.
9. Dùng `endl` sau mỗi dòng (flush liên tục) và không tắt đồng bộ với stdio.
10. `freopen` gắn cứng tên tệp, vi phạm yêu cầu "không phụ thuộc đường dẫn cá nhân".
11. `maxn` cố định `20005`, không kiểm tra biên.

---

**Có thể xóa thư mục này trước khi nộp** nếu không muốn kèm bản nháp — nhưng nên
giữ lại nếu báo cáo có trích dẫn mục 5.3.
