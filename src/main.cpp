// =============================================================================
//  main.cpp - Chương trình giải BÀI TOÁN KIỂM CHỨNG của chuyên đề 6.
//
//  ĐỊNH DẠNG VÀO (mọi chỉ số 1-based):
//      N Q
//      a_1 a_2 ... a_N
//      rồi Q dòng, mỗi dòng là một trong hai dạng:
//          1 L R V   -> cộng V vào mọi phần tử trong [L, R]
//          2 L R     -> in ra tổng các phần tử trong [L, R]
//
//  ĐỊNH DẠNG RA:
//      Mỗi truy vấn loại 2 in một dòng chứa một số nguyên 64-bit.
//
//  CÁCH DÙNG:
//      segtree                              # đọc stdin, ghi stdout
//      segtree in.txt out.txt               # đọc/ghi tệp
//      segtree --engine=naive in.txt out.txt# chạy lời giải đối chiếu O(N)
//      segtree --time in.txt out.txt        # in thời gian xử lý ra stderr
//
//  Không có đường dẫn cố định nào trong mã nguồn.
// =============================================================================
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

#include "naive.hpp"
#include "segment_tree.hpp"

namespace {

// ---------------------------------------------------------------------------
//  Bộ đọc số nguyên nhanh: nạp toàn bộ luồng vào bộ nhớ rồi phân tích tại chỗ.
//  Cần thiết vì phần thực nghiệm chạy tới hàng triệu thao tác.
// ---------------------------------------------------------------------------
class FastScanner {
public:
    explicit FastScanner(std::istream& in) {
        buf_.assign(std::istreambuf_iterator<char>(in),
                    std::istreambuf_iterator<char>());
        pos_ = 0;
    }

    bool read_ll(long long& out) {
        while (pos_ < buf_.size() &&
               (buf_[pos_] == ' ' || buf_[pos_] == '\n' ||
                buf_[pos_] == '\r' || buf_[pos_] == '\t'))
            ++pos_;
        if (pos_ >= buf_.size()) return false;
        bool neg = false;
        if (buf_[pos_] == '-') { neg = true; ++pos_; }
        else if (buf_[pos_] == '+') { ++pos_; }
        if (pos_ >= buf_.size() || buf_[pos_] < '0' || buf_[pos_] > '9') return false;
        long long x = 0;
        while (pos_ < buf_.size() && buf_[pos_] >= '0' && buf_[pos_] <= '9')
            x = x * 10 + (buf_[pos_++] - '0');
        out = neg ? -x : x;
        return true;
    }

    long long must_read_ll(const char* what) {
        long long x = 0;
        if (!read_ll(x)) {
            std::cerr << "Loi dinh dang du lieu vao: thieu " << what << "\n";
            std::exit(2);
        }
        return x;
    }

private:
    std::string buf_;
    std::size_t pos_ = 0;
};

// Ghi số nguyên nhanh vào bộ đệm chuỗi.
void append_ll(std::string& out, long long x) {
    char tmp[24];
    int len = 0;
    if (x == 0) { tmp[len++] = '0'; }
    else {
        bool neg = x < 0;
        unsigned long long u = neg ? (0ULL - static_cast<unsigned long long>(x))
                                   : static_cast<unsigned long long>(x);
        while (u) { tmp[len++] = static_cast<char>('0' + u % 10); u /= 10; }
        if (neg) tmp[len++] = '-';
    }
    while (len) out.push_back(tmp[--len]);
    out.push_back('\n');
}

struct Options {
    std::string input_path;
    std::string output_path;
    bool use_naive = false;
    bool show_time = false;
};

[[noreturn]] void usage_and_exit(const char* prog, int code) {
    std::cerr <<
        "Cach dung: " << prog << " [tuy_chon] [tep_vao] [tep_ra]\n"
        "  --engine=segtree|naive   Chon cai dat (mac dinh: segtree)\n"
        "  --time                   In thoi gian xu ly ra stderr\n"
        "  -h, --help               Hien thi tro giup\n"
        "Khong truyen tep_vao/tep_ra thi doc stdin va ghi stdout.\n";
    std::exit(code);
}

Options parse_args(int argc, char** argv) {
    Options o;
    std::vector<std::string> positional;
    for (int i = 1; i < argc; ++i) {
        std::string_view a(argv[i]);
        if (a == "-h" || a == "--help") usage_and_exit(argv[0], 0);
        else if (a == "--time") o.show_time = true;
        else if (a == "--engine=naive") o.use_naive = true;
        else if (a == "--engine=segtree") o.use_naive = false;
        else if (!a.empty() && a[0] == '-') {
            std::cerr << "Tuy chon khong hop le: " << a << "\n";
            usage_and_exit(argv[0], 2);
        } else positional.emplace_back(a);
    }
    if (positional.size() > 2) usage_and_exit(argv[0], 2);
    if (positional.size() >= 1) o.input_path = positional[0];
    if (positional.size() >= 2) o.output_path = positional[1];
    return o;
}

// Khuôn xử lý dùng chung cho cả hai cài đặt (segtree và naive).
template <class Engine>
void run(FastScanner& sc, std::string& out, long long n, long long q,
         const std::vector<long long>& a) {
    Engine tree(a);
    for (long long t = 0; t < q; ++t) {
        long long type = sc.must_read_ll("loai truy van");
        if (type == 1) {
            long long l = sc.must_read_ll("L");
            long long r = sc.must_read_ll("R");
            long long v = sc.must_read_ll("V");
            if (l < 1 || r > n || l > r) {
                std::cerr << "Truy van cap nhat co doan khong hop le: ["
                          << l << ", " << r << "]\n";
                std::exit(2);
            }
            tree.update_range(static_cast<int>(l - 1), static_cast<int>(r - 1), v);
        } else if (type == 2) {
            long long l = sc.must_read_ll("L");
            long long r = sc.must_read_ll("R");
            if (l < 1 || r > n || l > r) {
                std::cerr << "Truy van tong co doan khong hop le: ["
                          << l << ", " << r << "]\n";
                std::exit(2);
            }
            append_ll(out, tree.query_range(static_cast<int>(l - 1),
                                            static_cast<int>(r - 1)));
        } else {
            std::cerr << "Loai truy van khong hop le: " << type << "\n";
            std::exit(2);
        }
    }
}

}  // namespace

int main(int argc, char** argv) {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    Options opt = parse_args(argc, argv);

    std::ifstream fin;
    if (!opt.input_path.empty()) {
        fin.open(opt.input_path, std::ios::binary);
        if (!fin) {
            std::cerr << "Khong mo duoc tep vao: " << opt.input_path << "\n";
            return 2;
        }
    }
    std::istream& in = opt.input_path.empty() ? std::cin
                                              : static_cast<std::istream&>(fin);

    FastScanner sc(in);
    long long n = sc.must_read_ll("N");
    long long q = sc.must_read_ll("Q");
    if (n < 0 || q < 0) { std::cerr << "N va Q phai khong am\n"; return 2; }

    std::vector<long long> a(static_cast<std::size_t>(n));
    for (long long i = 0; i < n; ++i) a[static_cast<std::size_t>(i)] = sc.must_read_ll("a_i");

    std::string out;
    out.reserve(static_cast<std::size_t>(q) * 8 + 16);

    auto t0 = std::chrono::steady_clock::now();
    if (opt.use_naive) run<st::NaiveArray>(sc, out, n, q, a);
    else                run<st::SegmentTree>(sc, out, n, q, a);
    auto t1 = std::chrono::steady_clock::now();

    if (!opt.output_path.empty()) {
        std::ofstream fout(opt.output_path, std::ios::binary);
        if (!fout) {
            std::cerr << "Khong ghi duoc tep ra: " << opt.output_path << "\n";
            return 2;
        }
        fout.write(out.data(), static_cast<std::streamsize>(out.size()));
    } else {
        std::cout.write(out.data(), static_cast<std::streamsize>(out.size()));
        std::cout.flush();
    }

    if (opt.show_time) {
        double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
        std::cerr << "engine=" << (opt.use_naive ? "naive" : "segtree")
                  << " N=" << n << " Q=" << q
                  << " time_ms=" << ms << "\n";
    }
    return 0;
}
