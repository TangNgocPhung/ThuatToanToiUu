// =============================================================================
//  segment_tree_ext.hpp - Phần MỞ RỘNG của chuyên đề 6.
//
//  Hai biến thể minh họa rằng khung "apply / push_down / pull" là BẤT BIẾN,
//  chỉ có cặp (Value, Tag) và luật hợp thành thẻ là thay đổi:
//
//   1) SegmentTreeAssignAdd - Gán đoạn + Cộng đoạn, truy vấn tổng / min / max.
//      Thẻ hợp thành KHÔNG giao hoán: "gán" xóa sạch mọi "cộng" đứng trước nó.
//
//   2) SegmentTreeAffine    - Cập nhật affine x -> a*x + b (mod P), truy vấn
//      tổng (mod P). Đây là ví dụ chuẩn mực cho hợp thành thẻ không giao hoán.
//
//  Chuẩn: C++17. Header-only.
// =============================================================================
#ifndef ST_SEGMENT_TREE_EXT_HPP
#define ST_SEGMENT_TREE_EXT_HPP

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <vector>

namespace st {

namespace detail {

// ---------------------------------------------------------------------------
//  mul_mod - Nhân hai số rồi lấy dư, an toàn với tràn khi 0 <= a, b < m < 2^62.
//
//  Trên GCC/Clang dùng kiểu 128-bit (bọc __extension__ để giữ -Wpedantic sạch);
//  trình biên dịch không có 128-bit thì lùi về thuật toán nhân-dịch O(log b),
//  chậm hơn nhưng luôn đúng.
// ---------------------------------------------------------------------------
#if defined(__SIZEOF_INT128__)
__extension__ typedef unsigned __int128 uwide;

inline long long mul_mod(long long a, long long b, long long m) {
    return static_cast<long long>(static_cast<uwide>(a) * static_cast<uwide>(b) %
                                  static_cast<uwide>(m));
}
#else
inline long long mul_mod(long long a, long long b, long long m) {
    unsigned long long mm = static_cast<unsigned long long>(m);
    unsigned long long x = static_cast<unsigned long long>(a) % mm;
    unsigned long long y = static_cast<unsigned long long>(b) % mm;
    unsigned long long r = 0;
    while (y) {
        if (y & 1ULL) { r += x; if (r >= mm) r -= mm; }
        x <<= 1; if (x >= mm) x -= mm;
        y >>= 1;
    }
    return static_cast<long long>(r);
}
#endif

}  // namespace detail

// =============================================================================
//  1) SegmentTreeAssignAdd
//     Thao tác : assign_range(l, r, x)  -> gán mọi phần tử trong [l,r] bằng x
//                add_range(l, r, x)     -> cộng x vào mọi phần tử trong [l,r]
//     Truy vấn : sum(l, r), min(l, r), max(l, r)
// =============================================================================
class SegmentTreeAssignAdd {
public:
    // ------ Kiểu GIÁ TRỊ lưu tại nút -----------------------------------------
    struct Node {
        long long sum = 0;
        long long mn  = std::numeric_limits<long long>::max();
        long long mx  = std::numeric_limits<long long>::min();
    };

    // ------ Kiểu GIÁ TRỊ CẬP NHẬT (thẻ hoãn) ---------------------------------
    //  Ngữ nghĩa của một thẻ: "nếu has_assign thì GÁN assign_val trước,
    //  SAU ĐÓ cộng add_val". Mọi cặp (gán, cộng) đều rút gọn được về dạng này,
    //  nên thẻ có kích thước hằng số.
    struct Tag {
        bool      has_assign = false;
        long long assign_val = 0;
        long long add_val    = 0;

        bool is_identity() const noexcept { return !has_assign && add_val == 0; }
    };

    SegmentTreeAssignAdd() = default;
    explicit SegmentTreeAssignAdd(const std::vector<long long>& a) { build(a); }

    void build(const std::vector<long long>& a) {
        n_ = static_cast<int>(a.size());
        std::size_t sz = n_ > 0 ? 4 * static_cast<std::size_t>(n_) : 2;
        tree_.assign(sz, Node{});
        lazy_.assign(sz, Tag{});
        if (n_ > 0) build_rec(1, 0, n_ - 1, a);
    }

    int size() const noexcept { return n_; }

    void assign_range(int l, int r, long long x) {
        assert(0 <= l && l <= r && r < n_);
        Tag t; t.has_assign = true; t.assign_val = x; t.add_val = 0;
        update_rec(1, 0, n_ - 1, l, r, t);
    }

    void add_range(int l, int r, long long x) {
        assert(0 <= l && l <= r && r < n_);
        if (x == 0) return;
        Tag t; t.has_assign = false; t.add_val = x;
        update_rec(1, 0, n_ - 1, l, r, t);
    }

    Node query(int l, int r) {
        assert(0 <= l && l <= r && r < n_);
        return query_rec(1, 0, n_ - 1, l, r);
    }

    long long sum(int l, int r) { return query(l, r).sum; }
    long long min(int l, int r) { return query(l, r).mn; }
    long long max(int l, int r) { return query(l, r).mx; }

private:
    int n_ = 0;
    std::vector<Node> tree_;
    std::vector<Tag>  lazy_;

    static Node combine(const Node& a, const Node& b) {
        Node c;
        c.sum = a.sum + b.sum;
        c.mn  = std::min(a.mn, b.mn);
        c.mx  = std::max(a.mx, b.mx);
        return c;
    }

    static Node identity() { return Node{0, std::numeric_limits<long long>::max(),
                                             std::numeric_limits<long long>::min()}; }

    // Hợp thành thẻ: 'old' đã có sẵn trên nút, nay áp thêm 'nw' LÊN TRÊN.
    // Nếu nw có phép gán thì mọi thứ trước đó bị xóa -> nw thắng hoàn toàn.
    // Ngược lại nw chỉ cộng thêm -> giữ phép gán cũ, cộng dồn phần add.
    static Tag compose(const Tag& old_tag, const Tag& nw) {
        if (nw.has_assign) return nw;
        Tag res = old_tag;
        res.add_val += nw.add_val;
        return res;
    }

    void apply(int v, int len, const Tag& t) {
        if (t.is_identity()) return;
        if (t.has_assign) {
            tree_[v].sum = t.assign_val * static_cast<long long>(len);
            tree_[v].mn  = t.assign_val;
            tree_[v].mx  = t.assign_val;
        }
        if (t.add_val != 0) {
            tree_[v].sum += t.add_val * static_cast<long long>(len);
            tree_[v].mn  += t.add_val;
            tree_[v].mx  += t.add_val;
        }
        lazy_[v] = compose(lazy_[v], t);
    }

    void push_down(int v, int tl, int tm, int tr) {
        if (lazy_[v].is_identity()) return;
        apply(2 * v,     tm - tl + 1, lazy_[v]);
        apply(2 * v + 1, tr - tm,     lazy_[v]);
        lazy_[v] = Tag{};
    }

    void pull(int v) { tree_[v] = combine(tree_[2 * v], tree_[2 * v + 1]); }

    void build_rec(int v, int tl, int tr, const std::vector<long long>& a) {
        if (tl == tr) {
            long long x = a[static_cast<std::size_t>(tl)];
            tree_[v] = Node{x, x, x};
            return;
        }
        int tm = tl + (tr - tl) / 2;
        build_rec(2 * v, tl, tm, a);
        build_rec(2 * v + 1, tm + 1, tr, a);
        pull(v);
    }

    void update_rec(int v, int tl, int tr, int l, int r, const Tag& t) {
        if (r < tl || tr < l) return;
        if (l <= tl && tr <= r) { apply(v, tr - tl + 1, t); return; }
        int tm = tl + (tr - tl) / 2;
        push_down(v, tl, tm, tr);
        update_rec(2 * v, tl, tm, l, r, t);
        update_rec(2 * v + 1, tm + 1, tr, l, r, t);
        pull(v);
    }

    Node query_rec(int v, int tl, int tr, int l, int r) {
        if (r < tl || tr < l) return identity();
        if (l <= tl && tr <= r) return tree_[v];
        int tm = tl + (tr - tl) / 2;
        push_down(v, tl, tm, tr);
        return combine(query_rec(2 * v, tl, tm, l, r),
                       query_rec(2 * v + 1, tm + 1, tr, l, r));
    }
};

// =============================================================================
//  2) SegmentTreeAffine
//     Thao tác : apply_affine(l, r, a, b) -> x := a*x + b  (mod P) cho [l,r]
//     Truy vấn : sum(l, r) (mod P)
//
//     Hợp thành thẻ (điểm cốt lõi, KHÔNG giao hoán):
//         áp (a1,b1) trước rồi (a2,b2) sau
//             x -> a2*(a1*x + b1) + b2 = (a2*a1)*x + (a2*b1 + b2)
//     Thẻ đơn vị là (1, 0).
// =============================================================================
class SegmentTreeAffine {
public:
    struct Tag {
        long long a = 1;
        long long b = 0;
        bool is_identity() const noexcept { return a == 1 && b == 0; }
    };

    explicit SegmentTreeAffine(const std::vector<long long>& v,
                               long long mod = 1000000007LL)
        : mod_(mod) {
        assert(mod_ > 1);
        build(v);
    }

    void build(const std::vector<long long>& a) {
        n_ = static_cast<int>(a.size());
        std::size_t sz = n_ > 0 ? 4 * static_cast<std::size_t>(n_) : 2;
        tree_.assign(sz, 0);
        lazy_.assign(sz, Tag{});
        if (n_ > 0) build_rec(1, 0, n_ - 1, a);
    }

    int size() const noexcept { return n_; }
    long long mod() const noexcept { return mod_; }

    void apply_affine(int l, int r, long long a, long long b) {
        assert(0 <= l && l <= r && r < n_);
        Tag t{norm(a), norm(b)};
        if (t.is_identity()) return;
        update_rec(1, 0, n_ - 1, l, r, t);
    }

    // Tiện ích: gán đoạn = affine(0, x); cộng đoạn = affine(1, x).
    void assign_range(int l, int r, long long x) { apply_affine(l, r, 0, x); }
    void add_range(int l, int r, long long x)    { apply_affine(l, r, 1, x); }

    long long sum(int l, int r) {
        assert(0 <= l && l <= r && r < n_);
        return query_rec(1, 0, n_ - 1, l, r);
    }

private:
    int n_ = 0;
    long long mod_;
    std::vector<long long> tree_;
    std::vector<Tag> lazy_;

    long long norm(long long x) const { x %= mod_; return x < 0 ? x + mod_ : x; }
    long long mul(long long x, long long y) const { return detail::mul_mod(x, y, mod_); }
    long long add(long long x, long long y) const {
        long long s = x + y; return s >= mod_ ? s - mod_ : s;
    }

    static Tag compose(const Tag& first, const Tag& second, long long m) {
        // Áp 'first' trước, 'second' sau:
        //   x -> second.a * (first.a * x + first.b) + second.b
        Tag r;
        r.a = detail::mul_mod(second.a, first.a, m);
        r.b = detail::mul_mod(second.a, first.b, m) + second.b;
        if (r.b >= m) r.b -= m;
        return r;
    }

    void apply(int v, int len, const Tag& t) {
        if (t.is_identity()) return;
        // sum' = a*sum + b*len
        tree_[v] = add(mul(t.a, tree_[v]), mul(t.b, len % mod_));
        lazy_[v] = compose(lazy_[v], t, mod_);
    }

    void push_down(int v, int tl, int tm, int tr) {
        if (lazy_[v].is_identity()) return;
        apply(2 * v,     tm - tl + 1, lazy_[v]);
        apply(2 * v + 1, tr - tm,     lazy_[v]);
        lazy_[v] = Tag{};
    }

    void pull(int v) { tree_[v] = add(tree_[2 * v], tree_[2 * v + 1]); }

    void build_rec(int v, int tl, int tr, const std::vector<long long>& a) {
        if (tl == tr) { tree_[v] = norm(a[static_cast<std::size_t>(tl)]); return; }
        int tm = tl + (tr - tl) / 2;
        build_rec(2 * v, tl, tm, a);
        build_rec(2 * v + 1, tm + 1, tr, a);
        pull(v);
    }

    void update_rec(int v, int tl, int tr, int l, int r, const Tag& t) {
        if (r < tl || tr < l) return;
        if (l <= tl && tr <= r) { apply(v, tr - tl + 1, t); return; }
        int tm = tl + (tr - tl) / 2;
        push_down(v, tl, tm, tr);
        update_rec(2 * v, tl, tm, l, r, t);
        update_rec(2 * v + 1, tm + 1, tr, l, r, t);
        pull(v);
    }

    long long query_rec(int v, int tl, int tr, int l, int r) {
        if (r < tl || tr < l) return 0;
        if (l <= tl && tr <= r) return tree_[v];
        int tm = tl + (tr - tl) / 2;
        push_down(v, tl, tm, tr);
        return add(query_rec(2 * v, tl, tm, l, r),
                   query_rec(2 * v + 1, tm + 1, tr, l, r));
    }
};

}  // namespace st

#endif  // ST_SEGMENT_TREE_EXT_HPP
