// =============================================================================
//  stress_test.cpp - Kiểm thử ngẫu nhiên đối chiếu (stress test).
//
//  Sinh ngẫu nhiên hàng nghìn kịch bản, chạy song song CÂY PHÂN ĐOẠN và LỜI
//  GIẢI MẢNG THƯỜNG, dừng ngay ở kịch bản đầu tiên lệch nhau và ghi lại toàn
//  bộ dữ liệu tái lập được (hạt giống + tệp .in) vào thư mục hiện hành.
//
//  CÁCH DÙNG:
//      stress_test [--rounds=2000] [--seed=1] [--maxn=200] [--ops=500]
//                  [--maxval=1000000000] [--quiet]
//
//  Mã thoát: 0 = mọi vòng khớp, 1 = phát hiện sai lệch.
// =============================================================================
#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <random>
#include <string>
#include <string_view>
#include <vector>

#include "naive.hpp"
#include "segment_tree.hpp"

namespace {

struct Op {
    int type;  // 1 = cap nhat, 2 = truy van
    int l, r;
    long long v;
};

bool starts_with(std::string_view s, std::string_view p) {
    return s.size() >= p.size() && s.compare(0, p.size(), p) == 0;
}

void dump_case(const std::string& path, const std::vector<long long>& a,
               const std::vector<Op>& ops) {
    std::ofstream f(path, std::ios::binary);
    f << a.size() << ' ' << ops.size() << '\n';
    for (std::size_t i = 0; i < a.size(); ++i) f << a[i] << " \n"[i + 1 == a.size()];
    for (const Op& o : ops) {
        if (o.type == 1) f << "1 " << (o.l + 1) << ' ' << (o.r + 1) << ' ' << o.v << '\n';
        else             f << "2 " << (o.l + 1) << ' ' << (o.r + 1) << '\n';
    }
}

}  // namespace

int main(int argc, char** argv) {
    long long rounds = 2000, maxn = 200, ops_per_round = 500, maxval = 1000000000LL;
    std::uint64_t seed = 1;
    bool quiet = false;

    for (int i = 1; i < argc; ++i) {
        std::string_view a(argv[i]);
        if      (starts_with(a, "--rounds=")) rounds        = std::atoll(argv[i] + 9);
        else if (starts_with(a, "--seed="))   seed          = std::strtoull(argv[i] + 7, nullptr, 10);
        else if (starts_with(a, "--maxn="))   maxn          = std::atoll(argv[i] + 7);
        else if (starts_with(a, "--ops="))    ops_per_round = std::atoll(argv[i] + 6);
        else if (starts_with(a, "--maxval=")) maxval        = std::atoll(argv[i] + 9);
        else if (a == "--quiet")              quiet         = true;
        else { std::cerr << "Tuy chon khong hop le: " << a << "\n"; return 2; }
    }

    std::cout << "Stress test: rounds=" << rounds << " maxn=" << maxn
              << " ops=" << ops_per_round << " seed=" << seed << "\n";

    for (long long round = 0; round < rounds; ++round) {
        // Hạt giống suy ra từ (seed, round) nên mọi vòng đều tái lập được.
        std::uint64_t round_seed = seed * 1000003ULL + static_cast<std::uint64_t>(round);
        std::mt19937_64 rng(round_seed);

        int n = 1 + static_cast<int>(rng() % static_cast<std::uint64_t>(maxn));
        std::uniform_int_distribution<long long> dval(-maxval, maxval);

        std::vector<long long> a(static_cast<std::size_t>(n));
        for (auto& x : a) x = dval(rng);

        st::SegmentTree tree(a);
        st::NaiveArray  ref(a);
        std::vector<Op> ops;
        ops.reserve(static_cast<std::size_t>(ops_per_round));

        for (long long k = 0; k < ops_per_round; ++k) {
            int l = static_cast<int>(rng() % static_cast<std::uint64_t>(n));
            int r = static_cast<int>(rng() % static_cast<std::uint64_t>(n));
            if (l > r) std::swap(l, r);
            // Thỉnh thoảng ép các dạng biên: toàn đoạn, một điểm, tiền tố, hậu tố.
            switch (rng() % 8) {
                case 0: l = 0; r = n - 1; break;
                case 1: r = l;            break;
                case 2: l = 0;            break;
                case 3: r = n - 1;        break;
                default: break;
            }

            if (rng() % 2 == 0) {
                long long v = dval(rng);
                ops.push_back({1, l, r, v});
                tree.update_range(l, r, v);
                ref.update_range(l, r, v);
            } else {
                ops.push_back({2, l, r, 0});
                long long got = tree.query_range(l, r);
                long long want = ref.query_range(l, r);
                if (got != want) {
                    std::cout << "\n*** SAI LECH ***\n"
                              << "round      = " << round << "\n"
                              << "round_seed = " << round_seed << "\n"
                              << "n          = " << n << "\n"
                              << "thao tac   = " << k << " (truy van [" << (l + 1)
                              << ", " << (r + 1) << "] theo chi so 1-based)\n"
                              << "segtree    = " << got << "\n"
                              << "naive      = " << want << "\n";
                    dump_case("stress_fail.in", a, ops);
                    std::cout << "Da ghi kich ban loi ra: stress_fail.in\n";
                    return 1;
                }
            }
        }

        if (tree.to_vector() != ref.to_vector()) {
            std::cout << "\n*** SAI LECH O TRANG THAI CUOI CUNG ***\n"
                      << "round_seed = " << round_seed << " n = " << n << "\n";
            dump_case("stress_fail.in", a, ops);
            return 1;
        }

        if (!quiet && rounds >= 10 && (round + 1) % (rounds / 10) == 0)
            std::cout << "  ... " << (round + 1) << "/" << rounds << " vong da khop\n";
    }

    std::cout << "KET QUA: tat ca " << rounds
              << " vong ngau nhien deu khop voi loi giai mang thuong.\n";
    return 0;
}
