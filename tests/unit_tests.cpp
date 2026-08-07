// =============================================================================
//  unit_tests.cpp - Bộ kiểm thử của chuyên đề 6.
//
//  Bốn nhóm theo yêu cầu của đề:
//      [CO BAN]     hành vi đúng trên ví dụ nhỏ, kiểm tra được bằng tay.
//      [BIEN]       N=1, cập nhật toàn đoạn, một điểm, N không là lũy thừa 2,
//                   giá trị âm, cập nhật rỗng, biên 64-bit.
//      [DOI KHANG]  các mẫu truy vấn ép cây phải push_down liên tục, đoạn lệch
//                   trái/phải cực đoan, nhiều đoạn chồng lấn.
//      [NGAU NHIEN] đối chiếu với lời giải mảng thường trên dữ liệu ngẫu nhiên.
//
//  Biên dịch:  g++ -std=c++17 -O2 -I../src unit_tests.cpp -o unit_tests
//  Thoát mã 0 nếu mọi kiểm thử đạt, 1 nếu có kiểm thử hỏng.
// =============================================================================
#include <algorithm>
#include <cstdint>
#include <iostream>
#include <numeric>
#include <random>
#include <string>
#include <vector>

#include "naive.hpp"
#include "segment_tree.hpp"
#include "segment_tree_ext.hpp"

namespace {

int g_passed = 0;
int g_failed = 0;
std::string g_group;

void group(const std::string& name) {
    g_group = name;
    std::cout << "\n=== " << name << " ===\n";
}

void check(bool ok, const std::string& what) {
    if (ok) { ++g_passed; std::cout << "  [ OK ] " << what << "\n"; }
    else    { ++g_failed; std::cout << "  [FAIL] " << what << "\n"; }
}

template <class T>
void check_eq(const T& got, const T& want, const std::string& what) {
    if (got == want) { ++g_passed; std::cout << "  [ OK ] " << what << "\n"; }
    else {
        ++g_failed;
        std::cout << "  [FAIL] " << what << "  (nhan duoc " << got
                  << ", mong doi " << want << ")\n";
    }
}

// ---------------------------------------------------------------------------
//  [CO BAN]
// ---------------------------------------------------------------------------
void test_basic() {
    group("CO BAN");

    // Mảng mẫu, kiểm tra được bằng tay: a = [1, 2, 8, 9, 3]
    std::vector<long long> a = {1, 2, 8, 9, 3};
    st::SegmentTree t(a);

    check_eq(t.query_range(0, 4), 23LL, "tong toan mang = 23");
    check_eq(t.query_range(1, 3), 19LL, "tong a[1..3] = 2+8+9 = 19");
    check_eq(t.query_range(2, 3), 17LL, "tong a[2..3] = 8+9 = 17");
    check_eq(t.query_range(0, 0), 1LL,  "tong mot phan tu a[0] = 1");
    check_eq(t.query_range(4, 4), 3LL,  "tong mot phan tu a[4] = 3");

    // Cộng 5 vào [1, 3] -> a = [1, 7, 13, 14, 3]
    t.update_range(1, 3, 5);
    check_eq(t.query_range(0, 4), 38LL, "sau khi cong 5 vao [1,3]: tong = 38");
    check_eq(t.query_range(1, 3), 34LL, "sau khi cong 5 vao [1,3]: a[1..3] = 34");
    check_eq(t.query_range(0, 1), 8LL,  "sau khi cong 5 vao [1,3]: a[0..1] = 8");
    check_eq(t.at(2), 13LL,             "sau khi cong 5 vao [1,3]: a[2] = 13");

    std::vector<long long> want = {1, 7, 13, 14, 3};
    check(t.to_vector() == want, "trai phang cay khop mang mong doi");

    // Cộng chồng lên: cộng -3 vào [0, 2] -> a = [-2, 4, 10, 14, 3]
    t.update_range(0, 2, -3);
    std::vector<long long> want2 = {-2, 4, 10, 14, 3};
    check(t.to_vector() == want2, "cap nhat chong lan cho ket qua dung");
    check_eq(t.query_range(0, 4), 29LL, "tong sau hai cap nhat = 29");
}

// ---------------------------------------------------------------------------
//  [BIEN]
// ---------------------------------------------------------------------------
void test_edge() {
    group("BIEN");

    // N = 1
    {
        st::SegmentTree t(std::vector<long long>{42});
        check_eq(t.query_range(0, 0), 42LL, "N=1: truy van phan tu duy nhat");
        t.update_range(0, 0, -100);
        check_eq(t.query_range(0, 0), -58LL, "N=1: cap nhat phan tu duy nhat");
        check_eq(t.size(), 1, "N=1: size() = 1");
    }

    // Cây rỗng
    {
        st::SegmentTree t{};
        check(t.empty(), "N=0: cay rong khong sap chuong trinh");
    }

    // N không phải lũy thừa của 2
    for (int n : {3, 5, 6, 7, 9, 11, 13, 100, 1000, 4097}) {
        std::vector<long long> a(static_cast<std::size_t>(n));
        std::iota(a.begin(), a.end(), 1LL);  // a = 1, 2, ..., n
        st::SegmentTree t(a);
        long long want = 1LL * n * (n + 1) / 2;
        bool ok = (t.query_range(0, n - 1) == want);
        // cập nhật toàn đoạn rồi kiểm tra lại
        t.update_range(0, n - 1, 7);
        ok = ok && (t.query_range(0, n - 1) == want + 7LL * n);
        // truy vấn mọi tiền tố
        long long run = 0;
        for (int i = 0; i < n; ++i) {
            run += (i + 1) + 7;
            if (t.query_range(0, i) != run) { ok = false; break; }
        }
        check(ok, "N=" + std::to_string(n) + " (khong la luy thua cua 2)");
    }

    // Cập nhật toàn đoạn nhiều lần
    {
        int n = 1000;
        std::vector<long long> a(static_cast<std::size_t>(n), 0);
        st::SegmentTree t(a);
        for (int k = 0; k < 100; ++k) t.update_range(0, n - 1, 1);
        check_eq(t.query_range(0, n - 1), 100LL * n, "100 lan cap nhat toan doan");
        check_eq(t.at(500), 100LL, "gia tri mot phan tu sau 100 lan cap nhat toan doan");
    }

    // Cập nhật một điểm
    {
        int n = 64;
        st::SegmentTree t(std::vector<long long>(static_cast<std::size_t>(n), 0));
        for (int i = 0; i < n; ++i) t.update_range(i, i, i);
        check_eq(t.query_range(0, n - 1), 1LL * (n - 1) * n / 2, "cap nhat tung diem");
        check_eq(t.query_range(10, 10), 10LL, "truy van mot diem sau cap nhat diem");
    }

    // Giá trị âm
    {
        std::vector<long long> a = {-5, -4, -3, -2, -1};
        st::SegmentTree t(a);
        check_eq(t.query_range(0, 4), -15LL, "tong cac gia tri am");
        t.update_range(0, 4, -10);
        check_eq(t.query_range(0, 4), -65LL, "cong them gia tri am vao toan doan");
        // a = {-15, -14, -13, -12, -11}; cong 100 vao [1,2] -> 86 + 87 = 173
        t.update_range(1, 2, 100);
        check_eq(t.query_range(1, 2), 173LL, "cong gia tri duong len doan am");
    }

    // Cập nhật rỗng (V = 0) không được làm hỏng cây
    {
        std::vector<long long> a = {1, 2, 3, 4, 5};
        st::SegmentTree t(a);
        for (int k = 0; k < 50; ++k) t.update_range(0, 4, 0);
        check_eq(t.query_range(0, 4), 15LL, "cap nhat V=0 khong doi ket qua");
    }

    // Biên 64-bit: N = 200000, a_i = 10^9, cộng 10^9 mười lần vào toàn đoạn.
    // Tổng mong đợi = 200000 * (10^9 + 10 * 10^9) = 2.2 * 10^15  < 9.22 * 10^18.
    {
        const int n = 200000;
        std::vector<long long> a(static_cast<std::size_t>(n), 1000000000LL);
        st::SegmentTree t(a);
        for (int k = 0; k < 10; ++k) t.update_range(0, n - 1, 1000000000LL);
        long long want = 1LL * n * 11000000000LL;
        check_eq(t.query_range(0, n - 1), want, "tong lon 2.2e15 khong tran 64-bit");
        check_eq(t.at(0), 11000000000LL, "gia tri phan tu 1.1e10 (vuot 32-bit)");
    }

    // Đối xứng âm - dương triệt tiêu
    {
        const int n = 100000;
        st::SegmentTree t(std::vector<long long>(static_cast<std::size_t>(n), 0));
        t.update_range(0, n - 1,  1000000000LL);
        t.update_range(0, n - 1, -1000000000LL);
        check_eq(t.query_range(0, n - 1), 0LL, "cong roi tru trieu tieu ve 0");
    }
}

// ---------------------------------------------------------------------------
//  [DOI KHANG] - các mẫu cố ý làm khó lazy propagation
// ---------------------------------------------------------------------------
void test_adversarial() {
    group("DOI KHANG");

    // (A) HỒI QUY: lỗi "cây lai" - build gộp bằng tổng nhưng query gộp bằng min.
    //     Đoạn [2,3] (0-based) đúng bằng MỘT nút trong của cây n=5, nên bản cài
    //     đặt lỗi trả về t[nut] = 17 cho truy vấn min thay vì 8.
    {
        std::vector<long long> a = {1, 2, 8, 9, 3};
        st::SegmentTree t(a);
        check_eq(t.query_range(2, 3), 17LL, "hoi quy: truy van tong tren nut trong = 17");

        st::SegmentTreeAssignAdd m(a);
        check_eq(m.min(2, 3), 8LL,  "hoi quy: truy van min tren cung doan = 8");
        check_eq(m.max(2, 3), 9LL,  "hoi quy: truy van max tren cung doan = 9");
        check_eq(m.sum(2, 3), 17LL, "hoi quy: tong va min khong duoc lan lon");
    }

    // (B) Ép push_down tối đa: cập nhật toàn đoạn rồi truy vấn từng phần tử.
    {
        const int n = 1023;  // 2^10 - 1, cây lệch
        st::SegmentTree t(std::vector<long long>(static_cast<std::size_t>(n), 0));
        st::NaiveArray  r(std::vector<long long>(static_cast<std::size_t>(n), 0));
        for (int k = 1; k <= 20; ++k) { t.update_range(0, n - 1, k); r.update_range(0, n - 1, k); }
        bool ok = true;
        for (int i = 0; i < n && ok; ++i) ok = (t.at(i) == r.at(i));
        check(ok, "cap nhat toan doan xen ke truy van tung diem (ep push_down)");
    }

    // (C) Đoạn lồng nhau thu hẹp dần rồi truy vấn ngược ra ngoài.
    {
        const int n = 512;
        st::SegmentTree t(std::vector<long long>(static_cast<std::size_t>(n), 0));
        st::NaiveArray  r(std::vector<long long>(static_cast<std::size_t>(n), 0));
        int lo = 0, hi = n - 1;
        long long v = 1;
        while (lo <= hi) {
            t.update_range(lo, hi, v); r.update_range(lo, hi, v);
            ++lo; --hi; ++v;
        }
        bool ok = true;
        for (int i = 0; i < n && ok; ++i) ok = (t.query_range(0, i) == r.query_range(0, i));
        check(ok, "cac doan long nhau thu hep dan + truy van tien to");
    }

    // (D) Đoạn lệch cực đoan: chỉ tiền tố, rồi chỉ hậu tố.
    {
        const int n = 777;
        st::SegmentTree t(std::vector<long long>(static_cast<std::size_t>(n), 0));
        st::NaiveArray  r(std::vector<long long>(static_cast<std::size_t>(n), 0));
        for (int i = 0; i < n; ++i) { t.update_range(0, i, 1); r.update_range(0, i, 1); }
        for (int i = 0; i < n; ++i) { t.update_range(i, n - 1, -1); r.update_range(i, n - 1, -1); }
        bool ok = true;
        for (int i = 0; i < n && ok; ++i) ok = (t.at(i) == r.at(i));
        check(ok, "cap nhat toan bo tien to roi toan bo hau to");
    }

    // (E) Cập nhật nửa trái, truy vấn nửa phải (không được rò rỉ qua ranh giới).
    {
        const int n = 1000;
        st::SegmentTree t(std::vector<long long>(static_cast<std::size_t>(n), 0));
        for (int k = 0; k < 100; ++k) t.update_range(0, n / 2 - 1, 1000000);
        check_eq(t.query_range(n / 2, n - 1), 0LL, "cap nhat nua trai khong ro ri sang nua phai");
        // 500 phan tu, moi phan tu nhan 100 lan cong 10^6 -> 500 * 10^8 = 5 * 10^10
        check_eq(t.query_range(0, n / 2 - 1), 50000000000LL, "tong nua trai dung");
    }

    // (F) Hai đoạn chỉ giao nhau đúng một phần tử.
    {
        std::vector<long long> a(101, 0);
        st::SegmentTree t(a);
        t.update_range(0, 50, 3);
        t.update_range(50, 100, 5);
        check_eq(t.at(50), 8LL,  "phan tu giao cua hai doan nhan ca hai cap nhat");
        check_eq(t.at(49), 3LL,  "phan tu chi thuoc doan thu nhat");
        check_eq(t.at(51), 5LL,  "phan tu chi thuoc doan thu hai");
        check_eq(t.query_range(0, 100), 3LL * 51 + 5LL * 51, "tong sau hai doan giao nhau");
    }
}

// ---------------------------------------------------------------------------
//  [NGAU NHIEN] - đối chiếu với lời giải mảng thường
// ---------------------------------------------------------------------------
void test_random_vs_naive() {
    group("NGAU NHIEN DOI CHIEU");

    const int rounds = 300;
    bool all_ok = true;
    std::string first_bad;

    for (int round = 0; round < rounds && all_ok; ++round) {
        std::mt19937_64 rng(static_cast<std::uint64_t>(round) * 2654435761ULL + 7);
        int n = 1 + static_cast<int>(rng() % 60);
        std::uniform_int_distribution<long long> dval(-1000, 1000);
        std::vector<long long> a(static_cast<std::size_t>(n));
        for (auto& x : a) x = dval(rng);

        st::SegmentTree tree(a);
        st::NaiveArray  ref(a);

        int ops = 200;
        for (int k = 0; k < ops; ++k) {
            int l = static_cast<int>(rng() % n);
            int r = static_cast<int>(rng() % n);
            if (l > r) std::swap(l, r);
            if (rng() % 2 == 0) {
                long long v = dval(rng);
                tree.update_range(l, r, v);
                ref.update_range(l, r, v);
            } else {
                long long got = tree.query_range(l, r), want = ref.query_range(l, r);
                if (got != want) {
                    all_ok = false;
                    first_bad = "round=" + std::to_string(round) +
                                " n=" + std::to_string(n) +
                                " query[" + std::to_string(l) + "," + std::to_string(r) +
                                "] got=" + std::to_string(got) +
                                " want=" + std::to_string(want);
                    break;
                }
            }
        }
        if (all_ok && tree.to_vector() != ref.to_vector()) {
            all_ok = false;
            first_bad = "round=" + std::to_string(round) + ": mang cuoi cung khac nhau";
        }
    }
    check(all_ok, all_ok ? (std::to_string(rounds) + " vong ngau nhien khop voi mang thuong")
                         : ("sai lech: " + first_bad));

    // Vòng thứ hai: N lớn hơn, đoạn dài, giá trị lớn.
    {
        std::mt19937_64 rng(987654321ULL);
        int n = 5000;
        std::uniform_int_distribution<long long> dval(-1000000000LL, 1000000000LL);
        std::vector<long long> a(static_cast<std::size_t>(n));
        for (auto& x : a) x = dval(rng);
        st::SegmentTree tree(a);
        st::NaiveArray  ref(a);
        bool ok = true;
        for (int k = 0; k < 3000 && ok; ++k) {
            int l = static_cast<int>(rng() % n), r = static_cast<int>(rng() % n);
            if (l > r) std::swap(l, r);
            if (rng() % 2 == 0) {
                long long v = dval(rng);
                tree.update_range(l, r, v); ref.update_range(l, r, v);
            } else {
                ok = (tree.query_range(l, r) == ref.query_range(l, r));
            }
        }
        check(ok, "N=5000, gia tri toi 1e9, 3000 thao tac khop voi mang thuong");
    }
}

// ---------------------------------------------------------------------------
//  [MO RONG] - gán đoạn / min / max, và cập nhật affine
// ---------------------------------------------------------------------------
void test_extensions() {
    group("MO RONG: GAN DOAN + MIN/MAX");

    {
        std::vector<long long> a = {5, 3, 8, 1, 9, 2};
        st::SegmentTreeAssignAdd t(a);
        check_eq(t.sum(0, 5), 28LL, "tong ban dau");
        check_eq(t.min(0, 5), 1LL,  "min ban dau");
        check_eq(t.max(0, 5), 9LL,  "max ban dau");

        t.assign_range(1, 3, 4);              // a = 5 4 4 4 9 2
        check_eq(t.sum(0, 5), 28LL, "sau khi gan [1,3] = 4: tong");
        check_eq(t.min(0, 5), 2LL,  "sau khi gan [1,3] = 4: min");
        check_eq(t.max(0, 5), 9LL,  "sau khi gan [1,3] = 4: max");

        t.add_range(0, 2, 10);                // a = 15 14 14 4 9 2
        check_eq(t.sum(0, 2), 43LL, "gan roi cong: tong [0,2]");
        check_eq(t.max(0, 5), 15LL, "gan roi cong: max");
        check_eq(t.min(0, 5), 2LL,  "gan roi cong: min");

        // Gán PHẢI xóa mọi phép cộng đứng trước nó (thẻ khong giao hoan).
        t.add_range(0, 5, 1000);
        t.assign_range(0, 5, 7);
        check_eq(t.sum(0, 5), 42LL, "gan xoa sach cong dung truoc (6 * 7 = 42)");
        check_eq(t.min(0, 5), 7LL,  "gan xoa sach cong dung truoc: min = 7");
    }

    // Đối chiếu ngẫu nhiên cho biến thể gán + cộng.
    {
        std::mt19937_64 rng(20240606ULL);
        bool ok = true;
        for (int round = 0; round < 100 && ok; ++round) {
            int n = 1 + static_cast<int>(rng() % 40);
            std::vector<long long> a(static_cast<std::size_t>(n));
            for (auto& x : a) x = static_cast<long long>(rng() % 201) - 100;
            st::SegmentTreeAssignAdd t(a);
            std::vector<long long> ref = a;
            for (int k = 0; k < 150 && ok; ++k) {
                int l = static_cast<int>(rng() % n), r = static_cast<int>(rng() % n);
                if (l > r) std::swap(l, r);
                int op = static_cast<int>(rng() % 3);
                long long v = static_cast<long long>(rng() % 201) - 100;
                if (op == 0) {
                    t.assign_range(l, r, v);
                    for (int i = l; i <= r; ++i) ref[static_cast<std::size_t>(i)] = v;
                } else if (op == 1) {
                    t.add_range(l, r, v);
                    for (int i = l; i <= r; ++i) ref[static_cast<std::size_t>(i)] += v;
                } else {
                    long long s = 0,
                              mn = *std::min_element(ref.begin() + l, ref.begin() + r + 1),
                              mx = *std::max_element(ref.begin() + l, ref.begin() + r + 1);
                    for (int i = l; i <= r; ++i) s += ref[static_cast<std::size_t>(i)];
                    auto got = t.query(l, r);
                    ok = (got.sum == s && got.mn == mn && got.mx == mx);
                }
            }
        }
        check(ok, "100 vong ngau nhien: gan + cong + sum/min/max khop mang thuong");
    }

    group("MO RONG: CAP NHAT AFFINE (mod p)");
    {
        const long long P = 1000000007LL;
        std::vector<long long> a = {1, 2, 3, 4, 5};
        st::SegmentTreeAffine t(a, P);
        check_eq(t.sum(0, 4), 15LL, "tong ban dau");

        t.apply_affine(0, 4, 2, 1);           // x -> 2x + 1 : 3 5 7 9 11
        check_eq(t.sum(0, 4), 35LL, "sau x -> 2x+1 tren toan doan");
        check_eq(t.sum(1, 2), 12LL, "sau x -> 2x+1 tren doan con [1,2]");

        t.apply_affine(1, 3, 0, 4);           // gán 4 :  3 4 4 4 11
        check_eq(t.sum(0, 4), 26LL, "affine(0,4) hoat dong nhu phep gan");

        // Thứ tự hợp thành thẻ KHÔNG giao hoán.
        st::SegmentTreeAffine u(std::vector<long long>{10}, P);
        u.apply_affine(0, 0, 2, 0);  // 10 -> 20
        u.apply_affine(0, 0, 1, 3);  // 20 -> 23
        st::SegmentTreeAffine w(std::vector<long long>{10}, P);
        w.apply_affine(0, 0, 1, 3);  // 10 -> 13
        w.apply_affine(0, 0, 2, 0);  // 13 -> 26
        check_eq(u.sum(0, 0), 23LL, "hop thanh the: nhan roi cong = 23");
        check_eq(w.sum(0, 0), 26LL, "hop thanh the: cong roi nhan = 26 (khong giao hoan)");
    }

    // Đối chiếu ngẫu nhiên cho affine.
    {
        const long long P = 998244353LL;
        std::mt19937_64 rng(11223344ULL);
        bool ok = true;
        for (int round = 0; round < 60 && ok; ++round) {
            int n = 1 + static_cast<int>(rng() % 30);
            std::vector<long long> a(static_cast<std::size_t>(n));
            for (auto& x : a) x = static_cast<long long>(rng() % P);
            st::SegmentTreeAffine t(a, P);
            std::vector<long long> ref = a;
            for (int k = 0; k < 120 && ok; ++k) {
                int l = static_cast<int>(rng() % n), r = static_cast<int>(rng() % n);
                if (l > r) std::swap(l, r);
                if (rng() % 2 == 0) {
                    long long ca = static_cast<long long>(rng() % P);
                    long long cb = static_cast<long long>(rng() % P);
                    t.apply_affine(l, r, ca, cb);
                    for (int i = l; i <= r; ++i) {
                        auto& x = ref[static_cast<std::size_t>(i)];
                        x = (st::detail::mul_mod(ca, x, P) + cb) % P;
                    }
                } else {
                    long long s = 0;
                    for (int i = l; i <= r; ++i) s = (s + ref[static_cast<std::size_t>(i)]) % P;
                    ok = (t.sum(l, r) == s);
                }
            }
        }
        check(ok, "60 vong ngau nhien: affine mod 998244353 khop mang thuong");
    }
}

}  // namespace

int main() {
    std::cout << "Bo kiem thu chuyen de 6 - Segment Tree + Lazy Propagation\n";
#ifdef ST_CHECK_OVERFLOW
    std::cout << "(bien dich voi ST_CHECK_OVERFLOW: co kiem tra tran 64-bit)\n";
#endif

    test_basic();
    test_edge();
    test_adversarial();
    test_random_vs_naive();
    test_extensions();

    std::cout << "\n---------------------------------------------\n";
    std::cout << "Tong ket: " << g_passed << " dat, " << g_failed << " hong\n";
    return g_failed == 0 ? 0 : 1;
}
