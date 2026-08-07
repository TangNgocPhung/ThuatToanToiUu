// =============================================================================
//  segment_tree.hpp
//  Chuyên đề 6 - Cây phân đoạn (Segment Tree) và Lazy Propagation
//
//  Bài toán kiểm chứng:
//      - update_range(l, r, v) : cộng v vào MỌI phần tử trong đoạn [l, r]
//      - query_range(l, r)     : trả về TỔNG các phần tử trong đoạn [l, r]
//  Cả hai thao tác chạy trong O(log N).
//
//  Chuẩn: C++17. Header-only, không phụ thuộc thư viện ngoài.
// =============================================================================
#ifndef ST_SEGMENT_TREE_HPP
#define ST_SEGMENT_TREE_HPP

#include <cassert>
#include <cstddef>
#include <vector>

namespace st {

#ifdef ST_CHECK_OVERFLOW
namespace detail {
// Kiểu số rộng hơn 64-bit, CHỈ dùng cho khẳng định phát hiện tràn ở chế độ gỡ
// lỗi. __int128 là phần mở rộng của GCC/Clang nên phải bọc bằng __extension__
// để không vi phạm -Wpedantic; trình biên dịch nào không có thì lùi về
// long double (đủ để phát hiện tràn, tuy kém chính xác hơn).
#  if defined(__SIZEOF_INT128__)
__extension__ typedef __int128 wide_int;
#  else
typedef long double wide_int;
#  endif
constexpr long long kI64Min = -9223372036854775807LL - 1;
constexpr long long kI64Max =  9223372036854775807LL;
}  // namespace detail
#endif

// -----------------------------------------------------------------------------
//  Lớp SegmentTree: cây phân đoạn tổng đoạn + cập nhật đoạn (cộng) bằng lazy.
//
//  QUY ƯỚC CHỈ SỐ (PUBLIC API): 0-based, đoạn ĐÓNG [l, r], 0 <= l <= r < n.
//  Tầng nhập/xuất chịu trách nhiệm chuyển đổi từ 1-based sang 0-based.
//
//  TÁCH KIỂU DỮ LIỆU (yêu cầu của đề):
//      Value : kiểu của GIÁ TRỊ TỔNG HỢP lưu tại nút  -> tổng đoạn, 64-bit.
//      Tag   : kiểu của GIÁ TRỊ CẬP NHẬT hoãn lại     -> lượng cộng trên MỖI
//              phần tử của đoạn. Hai kiểu này TRÙNG NHAU về mặt biểu diễn
//              (đều là long long) nhưng KHÁC NHAU về ngữ nghĩa, nên được đặt
//              tên riêng: Value là "tổng của cả đoạn", Tag là "cộng cho một
//              phần tử". Nhầm lẫn hai khái niệm này là nguồn lỗi phổ biến nhất.
// -----------------------------------------------------------------------------
class SegmentTree {
public:
    using Value = long long;  // tổng đoạn (64-bit theo yêu cầu của đề)
    using Tag   = long long;  // lượng cộng cho mỗi phần tử trong đoạn

    // Phần tử trung hòa của phép gộp: combine(x, IDENTITY) == x.
    static constexpr Value VALUE_IDENTITY = 0;
    // Thẻ rỗng: "không có cập nhật nào đang chờ".
    static constexpr Tag TAG_IDENTITY = 0;

    SegmentTree() = default;

    explicit SegmentTree(const std::vector<Value>& a) { build(a); }

    // -------------------------------------------------------------------------
    //  build - Dựng cây từ mảng a. Độ phức tạp O(N) thời gian, O(N) bộ nhớ.
    //
    //  Vì sao cấp phát 4N?
    //      Cây phân đoạn đệ quy trên [0, n-1] KHÔNG phải cây nhị phân hoàn
    //      chỉnh khi n không là lũy thừa của 2. Chiều cao h = ceil(log2 n),
    //      chỉ số nút lớn nhất có thể đạt tới < 2^(h+1) <= 4n. Do đó 4N là cận
    //      trên an toàn phổ dụng cho mọi n >= 1 (với n = 1 ta vẫn cần >= 2 ô vì
    //      cây đánh số từ 1).
    // -------------------------------------------------------------------------
    void build(const std::vector<Value>& a) {
        n_ = static_cast<int>(a.size());
        tree_.assign(n_ > 0 ? 4 * static_cast<std::size_t>(n_) : 2, VALUE_IDENTITY);
        lazy_.assign(n_ > 0 ? 4 * static_cast<std::size_t>(n_) : 2, TAG_IDENTITY);
        if (n_ > 0) build_rec(ROOT, 0, n_ - 1, a);
    }

    int size() const noexcept { return n_; }
    bool empty() const noexcept { return n_ == 0; }

    // -------------------------------------------------------------------------
    //  update_range - Cộng v vào mọi phần tử trong [l, r]. O(log N).
    // -------------------------------------------------------------------------
    void update_range(int l, int r, Tag v) {
        assert(0 <= l && l <= r && r < n_ && "chi so doan khong hop le");
        if (v == TAG_IDENTITY) return;  // cập nhật rỗng: bỏ qua
        update_rec(ROOT, 0, n_ - 1, l, r, v);
    }

    // -------------------------------------------------------------------------
    //  query_range - Tổng các phần tử trong [l, r]. O(log N).
    // -------------------------------------------------------------------------
    Value query_range(int l, int r) {
        assert(0 <= l && l <= r && r < n_ && "chi so doan khong hop le");
        return query_rec(ROOT, 0, n_ - 1, l, r);
    }

    // Tiện ích cho kiểm thử: giá trị của một phần tử.
    Value at(int i) { return query_range(i, i); }

    // Tiện ích cho kiểm thử: trải phẳng cây thành mảng (đẩy hết lazy xuống lá).
    std::vector<Value> to_vector() {
        std::vector<Value> out(static_cast<std::size_t>(n_), 0);
        if (n_ > 0) flatten_rec(ROOT, 0, n_ - 1, out);
        return out;
    }

    // Số ô bộ nhớ thực sự cấp phát (dùng cho phần thực nghiệm).
    std::size_t memory_bytes() const noexcept {
        return tree_.capacity() * sizeof(Value) + lazy_.capacity() * sizeof(Tag);
    }

private:
    static constexpr int ROOT = 1;

    int n_ = 0;
    std::vector<Value> tree_;  // tree_[v] = tổng đoạn của nút v, ĐÃ tính lazy_[v]
    std::vector<Tag>   lazy_;  // lazy_[v] = cập nhật CHƯA đẩy xuống hai con của v

    static constexpr int left(int v) noexcept { return 2 * v; }
    static constexpr int right(int v) noexcept { return 2 * v + 1; }

    // -------------------------------------------------------------------------
    //  combine - Phép gộp (monoid). Đổi hàm này (và VALUE_IDENTITY) là chuyển
    //  được sang min/max/gcd... miễn phép gộp có tính kết hợp.
    // -------------------------------------------------------------------------
    static constexpr Value combine(Value a, Value b) noexcept { return a + b; }

    // -------------------------------------------------------------------------
    //  BẤT BIẾN CỦA CÂY (điểm mấu chốt của lazy propagation)
    //
    //    (I1) tree_[v] là giá trị gộp ĐÚNG của đoạn mà nút v quản lý, đã bao
    //         gồm mọi cập nhật đã tác động lên v, KỂ CẢ lazy_[v].
    //    (I2) lazy_[v] là cập nhật đã được áp cho tree_[v] nhưng CHƯA được áp
    //         cho hai cây con của v.
    //
    //  Hệ quả: đọc tree_[v] luôn hợp lệ; chỉ khi cần ĐI XUỐNG con mới phải
    //  push_down để khôi phục (I1) cho các con.
    // -------------------------------------------------------------------------

    // -------------------------------------------------------------------------
    //  apply - Áp thẻ t lên nút v quản lý đoạn có len phần tử.
    //          Cập nhật cả giá trị gộp lẫn thẻ hoãn của chính v.
    //
    //  Cộng v vào len phần tử làm tổng tăng thêm t * len.
    //  Hợp thành thẻ: phép "cộng" giao hoán và kết hợp nên chỉ cần cộng dồn.
    // -------------------------------------------------------------------------
    void apply(int v, int len, Tag t) {
#ifdef ST_CHECK_OVERFLOW
        // Kiểm tra tràn 64-bit ở chế độ gỡ lỗi (xem tests/unit_tests.cpp).
        {
            using W = detail::wide_int;
            W total = static_cast<W>(tree_[v]) + static_cast<W>(t) * static_cast<W>(len);
            assert(total >= static_cast<W>(detail::kI64Min) &&
                   total <= static_cast<W>(detail::kI64Max) &&
                   "TRAN 64-bit: tong doan vuot qua long long");
            W tag = static_cast<W>(lazy_[v]) + static_cast<W>(t);
            assert(tag >= static_cast<W>(detail::kI64Min) &&
                   tag <= static_cast<W>(detail::kI64Max) &&
                   "TRAN 64-bit: the lazy vuot qua long long");
        }
#endif
        tree_[v] += t * static_cast<Value>(len);
        lazy_[v] += t;
    }

    // -------------------------------------------------------------------------
    //  push_down - Đẩy thẻ hoãn của v xuống hai con, khôi phục bất biến (I1)
    //              cho các con trước khi đệ quy xuống.
    //
    //  Nút v quản lý [tl, tr], điểm chia tm; con trái giữ [tl, tm] (tm-tl+1
    //  phần tử), con phải giữ [tm+1, tr] (tr-tm phần tử). Truyền độ dài tường
    //  minh để tránh tính nhầm khi n không là lũy thừa của 2.
    // -------------------------------------------------------------------------
    void push_down(int v, int tl, int tm, int tr) {
        if (lazy_[v] == TAG_IDENTITY) return;  // không có gì để đẩy
        apply(left(v),  tm - tl + 1, lazy_[v]);
        apply(right(v), tr - tm,     lazy_[v]);
        lazy_[v] = TAG_IDENTITY;
    }

    // pull - Tính lại giá trị nút cha từ hai con (sau khi con đã đúng).
    void pull(int v) { tree_[v] = combine(tree_[left(v)], tree_[right(v)]); }

    void build_rec(int v, int tl, int tr, const std::vector<Value>& a) {
        if (tl == tr) {                       // nút lá
            tree_[v] = a[static_cast<std::size_t>(tl)];
            return;
        }
        int tm = tl + (tr - tl) / 2;          // tránh tràn khi tl+tr lớn
        build_rec(left(v),  tl,     tm, a);
        build_rec(right(v), tm + 1, tr, a);
        pull(v);
    }

    // -------------------------------------------------------------------------
    //  update_rec - Ba trường hợp kinh điển:
    //    (1) [tl,tr] NẰM NGOÀI [l,r]        -> dừng, không làm gì.
    //    (2) [tl,tr] NẰM TRỌN trong [l,r]   -> apply rồi DỪNG (đây là chỗ lazy
    //        tiết kiệm: không đi sâu vào cây con).
    //    (3) GIAO MỘT PHẦN                  -> push_down rồi đệ quy hai con.
    // -------------------------------------------------------------------------
    void update_rec(int v, int tl, int tr, int l, int r, Tag t) {
        if (r < tl || tr < l) return;                       // (1) ngoài
        if (l <= tl && tr <= r) {                           // (2) phủ toàn bộ
            apply(v, tr - tl + 1, t);
            return;
        }
        int tm = tl + (tr - tl) / 2;                        // (3) giao một phần
        push_down(v, tl, tm, tr);
        update_rec(left(v),  tl,     tm, l, r, t);
        update_rec(right(v), tm + 1, tr, l, r, t);
        pull(v);
    }

    Value query_rec(int v, int tl, int tr, int l, int r) {
        if (r < tl || tr < l) return VALUE_IDENTITY;        // (1) ngoài
        if (l <= tl && tr <= r) return tree_[v];            // (2) phủ toàn bộ
        int tm = tl + (tr - tl) / 2;                        // (3) giao một phần
        push_down(v, tl, tm, tr);
        return combine(query_rec(left(v),  tl,     tm, l, r),
                       query_rec(right(v), tm + 1, tr, l, r));
    }

    void flatten_rec(int v, int tl, int tr, std::vector<Value>& out) {
        if (tl == tr) {
            out[static_cast<std::size_t>(tl)] = tree_[v];
            return;
        }
        int tm = tl + (tr - tl) / 2;
        push_down(v, tl, tm, tr);
        flatten_rec(left(v),  tl,     tm, out);
        flatten_rec(right(v), tm + 1, tr, out);
    }
};

}  // namespace st

#endif  // ST_SEGMENT_TREE_HPP
