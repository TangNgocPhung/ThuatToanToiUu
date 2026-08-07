// =============================================================================
//  naive.hpp - Lời giải đơn giản (mảng thường) dùng làm CHUẨN ĐỐI CHIẾU.
//
//  Cùng giao diện công khai với st::SegmentTree nhưng cài đặt tầm thường:
//      update_range : O(N)
//      query_range  : O(N)
//  Vì quá đơn giản nên gần như chắc chắn đúng; mọi sai khác giữa hai cài đặt
//  đều quy về lỗi của cây phân đoạn.
// =============================================================================
#ifndef ST_NAIVE_HPP
#define ST_NAIVE_HPP

#include <cassert>
#include <cstddef>
#include <vector>

namespace st {

class NaiveArray {
public:
    using Value = long long;
    using Tag   = long long;

    NaiveArray() = default;
    explicit NaiveArray(const std::vector<Value>& a) { build(a); }

    void build(const std::vector<Value>& a) { data_ = a; }

    int size() const noexcept { return static_cast<int>(data_.size()); }
    bool empty() const noexcept { return data_.empty(); }

    void update_range(int l, int r, Tag v) {
        assert(0 <= l && l <= r && r < size());
        for (int i = l; i <= r; ++i) data_[static_cast<std::size_t>(i)] += v;
    }

    Value query_range(int l, int r) const {
        assert(0 <= l && l <= r && r < size());
        Value s = 0;
        for (int i = l; i <= r; ++i) s += data_[static_cast<std::size_t>(i)];
        return s;
    }

    Value at(int i) const { return data_[static_cast<std::size_t>(i)]; }

    const std::vector<Value>& to_vector() const { return data_; }

private:
    std::vector<Value> data_;
};

}  // namespace st

#endif  // ST_NAIVE_HPP
