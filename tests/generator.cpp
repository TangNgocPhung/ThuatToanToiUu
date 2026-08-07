// =============================================================================
//  generator.cpp - Sinh dữ liệu vào hợp lệ cho bài toán kiểm chứng.
//
//  CÁCH DÙNG:
//      generator --n=1000 --q=1000 --seed=12345 [tuy_chon]
//
//  Tùy chọn:
//      --n=<int>        so phan tu N            (mac dinh 1000)
//      --q=<int>        so truy van Q           (mac dinh 1000)
//      --seed=<u64>     hat giong ngau nhien    (mac dinh 1)
//      --maxval=<ll>    |a_i| <= maxval         (mac dinh 1000000000)
//      --maxadd=<ll>    |V|   <= maxadd         (mac dinh 1000000000)
//      --pupd=<0..1>    ti le truy van cap nhat (mac dinh 0.5)
//      --mode=<mode>    dang doan sinh ra:
//                          mixed   - do dai doan ngau nhien (mac dinh)
//                          full    - moi truy van phu toan doan [1, N]
//                          point   - moi truy van la mot diem (L == R)
//                          small   - doan ngan (<= 8 phan tu)
//                          prefix  - luon bat dau tu 1
//                          suffix  - luon ket thuc tai N
//      --out=<path>     ghi ra tep thay vi stdout
//
//  Mọi dữ liệu sinh ra đều TÁI LẬP ĐƯỢC từ (seed, n, q, mode).
// =============================================================================
#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <random>
#include <string>
#include <string_view>

namespace {

enum class Mode { Mixed, Full, Point, Small, Prefix, Suffix };

Mode parse_mode(std::string_view s) {
    if (s == "mixed")  return Mode::Mixed;
    if (s == "full")   return Mode::Full;
    if (s == "point")  return Mode::Point;
    if (s == "small")  return Mode::Small;
    if (s == "prefix") return Mode::Prefix;
    if (s == "suffix") return Mode::Suffix;
    std::cerr << "mode khong hop le: " << s << "\n";
    std::exit(2);
}

bool starts_with(std::string_view s, std::string_view p) {
    return s.size() >= p.size() && s.compare(0, p.size(), p) == 0;
}

}  // namespace

int main(int argc, char** argv) {
    long long n = 1000, q = 1000, maxval = 1000000000LL, maxadd = 1000000000LL;
    std::uint64_t seed = 1;
    double pupd = 0.5;
    Mode mode = Mode::Mixed;
    std::string out_path;

    for (int i = 1; i < argc; ++i) {
        std::string_view a(argv[i]);
        if      (starts_with(a, "--n="))      n      = std::atoll(argv[i] + 4);
        else if (starts_with(a, "--q="))      q      = std::atoll(argv[i] + 4);
        else if (starts_with(a, "--seed="))   seed   = std::strtoull(argv[i] + 7, nullptr, 10);
        else if (starts_with(a, "--maxval=")) maxval = std::atoll(argv[i] + 9);
        else if (starts_with(a, "--maxadd=")) maxadd = std::atoll(argv[i] + 9);
        else if (starts_with(a, "--pupd="))   pupd   = std::atof(argv[i] + 7);
        else if (starts_with(a, "--mode="))   mode   = parse_mode(a.substr(7));
        else if (starts_with(a, "--out="))    out_path = std::string(a.substr(6));
        else { std::cerr << "Tuy chon khong hop le: " << a << "\n"; return 2; }
    }
    if (n < 1 || q < 0) { std::cerr << "N phai >= 1 va Q phai >= 0\n"; return 2; }

    std::ofstream fout;
    if (!out_path.empty()) {
        fout.open(out_path, std::ios::binary);
        if (!fout) { std::cerr << "Khong ghi duoc: " << out_path << "\n"; return 2; }
    }
    std::ostream& os = out_path.empty() ? std::cout
                                        : static_cast<std::ostream&>(fout);

    std::mt19937_64 rng(seed);
    std::uniform_int_distribution<long long> dval(-maxval, maxval);
    std::uniform_int_distribution<long long> dadd(-maxadd, maxadd);
    std::uniform_int_distribution<long long> dpos(1, n);
    std::uniform_real_distribution<double>   dprob(0.0, 1.0);

    std::string buf;
    buf.reserve(static_cast<std::size_t>(q) * 24 + static_cast<std::size_t>(n) * 12 + 64);

    buf += std::to_string(n); buf += ' '; buf += std::to_string(q); buf += '\n';
    for (long long i = 0; i < n; ++i) {
        buf += std::to_string(dval(rng));
        buf += (i + 1 == n) ? '\n' : ' ';
    }
    if (n == 0) buf += '\n';

    for (long long t = 0; t < q; ++t) {
        long long l = 1, r = n;
        switch (mode) {
            case Mode::Full:
                l = 1; r = n; break;
            case Mode::Point:
                l = r = dpos(rng); break;
            case Mode::Small: {
                l = dpos(rng);
                long long len = 1 + static_cast<long long>(rng() % 8);
                r = std::min(n, l + len - 1);
                break;
            }
            case Mode::Prefix:
                l = 1; r = dpos(rng); break;
            case Mode::Suffix:
                l = dpos(rng); r = n; break;
            case Mode::Mixed:
            default:
                l = dpos(rng); r = dpos(rng);
                if (l > r) std::swap(l, r);
                break;
        }
        bool is_update = dprob(rng) < pupd;
        if (is_update) {
            buf += "1 "; buf += std::to_string(l); buf += ' ';
            buf += std::to_string(r); buf += ' ';
            buf += std::to_string(dadd(rng)); buf += '\n';
        } else {
            buf += "2 "; buf += std::to_string(l); buf += ' ';
            buf += std::to_string(r); buf += '\n';
        }
    }

    os.write(buf.data(), static_cast<std::streamsize>(buf.size()));
    return 0;
}
