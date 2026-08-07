// =============================================================================
//  bench.cpp - Chương trình THỰC NGHIỆM đo thời gian của chuyên đề 6.
//
//  Đo thời gian theo SỐ THAO TÁC và theo KÍCH THƯỚC N, cho bốn kiểu tải:
//      mixed  : 50% cập nhật đoạn, 50% truy vấn đoạn, đoạn ngẫu nhiên
//      update : 100% cập nhật đoạn
//      query  : 100% truy vấn đoạn
//      point  : 100% thao tác trên đoạn một phần tử (L == R)
//      full   : 100% thao tác trên toàn đoạn [1, N] (trường hợp lazy có lợi nhất)
//
//  Kết quả in ra dạng CSV để đưa thẳng vào bảng trong báo cáo.
//
//  CÁCH DÙNG:
//      bench [--sizes=1000,10000,100000] [--ops=200000] [--repeat=3]
//            [--with-naive] [--out=results/benchmark.csv]
//
//  Ghi chú về phép đo:
//   - Mỗi cấu hình chạy `repeat` lần, báo cáo thời gian NHỎ NHẤT (ít nhiễu nhất
//     từ hệ điều hành) kèm giá trị trung bình.
//   - Chuỗi thao tác được SINH TRƯỚC và nạp sẵn vào bộ nhớ, nên thời gian đo
//     không lẫn chi phí sinh số ngẫu nhiên hay nhập/xuất.
//   - `checksum` là tổng của mọi kết quả truy vấn: vừa chặn trình tối ưu loại
//     bỏ vòng lặp, vừa xác nhận cây phân đoạn và mảng thường cho cùng kết quả.
// =============================================================================
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

#include "naive.hpp"
#include "segment_tree.hpp"

namespace {

struct Op { int type; int l, r; long long v; };

enum class Load { Mixed, Update, Query, Point, Full };

const char* load_name(Load w) {
    switch (w) {
        case Load::Mixed:  return "mixed";
        case Load::Update: return "update";
        case Load::Query:  return "query";
        case Load::Point:  return "point";
        case Load::Full:   return "full";
    }
    return "?";
}

std::vector<Op> make_ops(int n, long long q, Load w, std::uint64_t seed) {
    std::mt19937_64 rng(seed);
    std::uniform_int_distribution<long long> dval(-1000000000LL, 1000000000LL);
    std::vector<Op> ops;
    ops.reserve(static_cast<std::size_t>(q));
    for (long long k = 0; k < q; ++k) {
        int l, r;
        if (w == Load::Full)       { l = 0; r = n - 1; }
        else if (w == Load::Point) { l = r = static_cast<int>(rng() % static_cast<std::uint64_t>(n)); }
        else {
            l = static_cast<int>(rng() % static_cast<std::uint64_t>(n));
            r = static_cast<int>(rng() % static_cast<std::uint64_t>(n));
            if (l > r) std::swap(l, r);
        }
        int type;
        if (w == Load::Update)      type = 1;
        else if (w == Load::Query)  type = 2;
        else                        type = (rng() % 2 == 0) ? 1 : 2;
        ops.push_back({type, l, r, dval(rng)});
    }
    return ops;
}

template <class Engine>
struct RunResult { double build_ms; double ops_ms; long long checksum; };

template <class Engine>
RunResult<Engine> run_once(const std::vector<long long>& a, const std::vector<Op>& ops) {
    auto t0 = std::chrono::steady_clock::now();
    Engine tree(a);
    auto t1 = std::chrono::steady_clock::now();

    long long checksum = 0;
    for (const Op& o : ops) {
        if (o.type == 1) tree.update_range(o.l, o.r, o.v);
        else             checksum += tree.query_range(o.l, o.r);
    }
    auto t2 = std::chrono::steady_clock::now();

    return {std::chrono::duration<double, std::milli>(t1 - t0).count(),
            std::chrono::duration<double, std::milli>(t2 - t1).count(),
            checksum};
}

bool starts_with(std::string_view s, std::string_view p) {
    return s.size() >= p.size() && s.compare(0, p.size(), p) == 0;
}

std::vector<int> parse_sizes(std::string_view s) {
    std::vector<int> v;
    std::string cur;
    for (char c : s) {
        if (c == ',') { if (!cur.empty()) v.push_back(std::atoi(cur.c_str())); cur.clear(); }
        else cur.push_back(c);
    }
    if (!cur.empty()) v.push_back(std::atoi(cur.c_str()));
    return v;
}

std::string fmt(double x, int prec = 3) {
    std::ostringstream os;
    os << std::fixed << std::setprecision(prec) << x;
    return os.str();
}

}  // namespace

int main(int argc, char** argv) {
    std::vector<int> sizes = {1000, 10000, 100000, 1000000};
    long long ops_count = 200000;
    int repeat = 3;
    bool with_naive = false;
    // Chi chay loi giai mang thuong khi N*Q con duoi nguong nay, vi chi phi cua
    // no la O(N) moi thao tac; vuot nguong thi mot cau hinh se ngon hang phut.
    double naive_max_work = 2.5e10;
    std::string out_path;

    for (int i = 1; i < argc; ++i) {
        std::string_view a(argv[i]);
        if      (starts_with(a, "--sizes="))  sizes      = parse_sizes(a.substr(8));
        else if (starts_with(a, "--ops="))    ops_count  = std::atoll(argv[i] + 6);
        else if (starts_with(a, "--repeat=")) repeat     = std::atoi(argv[i] + 9);
        else if (a == "--with-naive")         with_naive = true;
        else if (starts_with(a, "--naive-max-work=")) naive_max_work = std::atof(argv[i] + 17);
        else if (starts_with(a, "--out="))    out_path   = std::string(a.substr(6));
        else { std::cerr << "Tuy chon khong hop le: " << a << "\n"; return 2; }
    }
    if (repeat < 1) repeat = 1;

    std::ostringstream csv;
    csv << "engine,workload,n,q,build_ms,ops_ms_min,ops_ms_avg,ns_per_op,"
           "tree_bytes,checksum\n";

    const Load loads[] = {Load::Mixed, Load::Update, Load::Query, Load::Point, Load::Full};

    std::cout << std::left
              << std::setw(9)  << "engine"
              << std::setw(9)  << "load"
              << std::setw(10) << "n"
              << std::setw(10) << "q"
              << std::setw(12) << "build_ms"
              << std::setw(12) << "ops_ms"
              << std::setw(12) << "ns/op" << "\n";
    std::cout << std::string(74, '-') << "\n";

    for (int n : sizes) {
        if (n < 1) continue;
        std::mt19937_64 rng(424242ULL + static_cast<std::uint64_t>(n));
        std::uniform_int_distribution<long long> dval(-1000000000LL, 1000000000LL);
        std::vector<long long> a(static_cast<std::size_t>(n));
        for (auto& x : a) x = dval(rng);

        for (Load w : loads) {
            std::vector<Op> ops = make_ops(n, ops_count, w, 991ULL + static_cast<std::uint64_t>(n));

            double best = 1e300, sum = 0, build_ms = 0;
            long long checksum = 0;
            std::size_t bytes = 0;
            for (int rep = 0; rep < repeat; ++rep) {
                auto r = run_once<st::SegmentTree>(a, ops);
                best = std::min(best, r.ops_ms);
                sum += r.ops_ms;
                build_ms = r.build_ms;
                checksum = r.checksum;
            }
            bytes = st::SegmentTree(a).memory_bytes();
            double ns_per_op = best * 1e6 / static_cast<double>(ops_count);

            csv << "segtree," << load_name(w) << ',' << n << ',' << ops_count << ','
                << fmt(build_ms) << ',' << fmt(best) << ',' << fmt(sum / repeat) << ','
                << fmt(ns_per_op, 1) << ',' << bytes << ',' << checksum << '\n';

            std::cout << std::left
                      << std::setw(9)  << "segtree"
                      << std::setw(9)  << load_name(w)
                      << std::setw(10) << n
                      << std::setw(10) << ops_count
                      << std::setw(12) << fmt(build_ms)
                      << std::setw(12) << fmt(best)
                      << std::setw(12) << fmt(ns_per_op, 1) << "\n";

            // Lời giải mảng thường chỉ chạy khi tổng chi phí O(N*Q) còn chấp nhận
            // được. Nó cũng chỉ lặp tối đa 2 lần vì mỗi lần đã rất tốn thời gian.
            if (with_naive &&
                static_cast<double>(n) * static_cast<double>(ops_count) <= naive_max_work) {
                int nrepeat = std::min(repeat, 2);
                double nbest = 1e300, nsum = 0, nbuild = 0;
                long long nchk = 0;
                for (int rep = 0; rep < nrepeat; ++rep) {
                    auto r = run_once<st::NaiveArray>(a, ops);
                    nbest = std::min(nbest, r.ops_ms);
                    nsum += r.ops_ms;
                    nbuild = r.build_ms;
                    nchk = r.checksum;
                }
                if (nchk != checksum)
                    std::cerr << "CANH BAO: checksum khac nhau (n=" << n << ", load="
                              << load_name(w) << "): segtree=" << checksum
                              << " naive=" << nchk << "\n";
                double nns = nbest * 1e6 / static_cast<double>(ops_count);
                csv << "naive," << load_name(w) << ',' << n << ',' << ops_count << ','
                    << fmt(nbuild) << ',' << fmt(nbest) << ',' << fmt(nsum / nrepeat) << ','
                    << fmt(nns, 1) << ',' << (sizeof(long long) * static_cast<std::size_t>(n))
                    << ',' << nchk << '\n';
                std::cout << std::left
                          << std::setw(9)  << "naive"
                          << std::setw(9)  << load_name(w)
                          << std::setw(10) << n
                          << std::setw(10) << ops_count
                          << std::setw(12) << fmt(nbuild)
                          << std::setw(12) << fmt(nbest)
                          << std::setw(12) << fmt(nns, 1) << "\n";
            }
        }
        std::cout << std::string(74, '-') << "\n";
    }

    if (!out_path.empty()) {
        std::ofstream f(out_path, std::ios::binary);
        if (!f) { std::cerr << "Khong ghi duoc: " << out_path << "\n"; return 2; }
        std::string s = csv.str();
        f.write(s.data(), static_cast<std::streamsize>(s.size()));
        std::cout << "Da ghi CSV: " << out_path << "\n";
    } else {
        std::cout << "\n--- CSV ---\n" << csv.str();
    }
    return 0;
}
