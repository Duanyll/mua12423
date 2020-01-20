#pragma once
#include <functional>
#include <vector>
#include <iostream>

#ifdef _MSC_VER
// ÈÃ VC ±Õ×ì
#define sprintf sprintf_s
#endif  // _MSC_VER

namespace mua {
namespace utils {

template <typename T>
struct mua_mod {
    T operator()(T a, T b) const { return a - std::floor(a / b) * b; }
};

template <typename T>
struct mua_pow {
    T operator()(T a, T b) const { return std::pow(a, b); }
};

template <typename T>
void merge_sort(int l, int r, std::vector<T>& arr, std::vector<T>& tmp,
                const std::function<bool(T, T)>& comp) {
    if (l >= r) return;
    int mid = (l + r) >> 1;
    merge_sort(l, mid, arr, tmp, comp);
    merge_sort(mid + 1, r, arr, tmp, comp);
    int i = l, j = mid + 1;
    int k = l;
    while (i <= mid && j <= r) {
        if (comp(arr[i], arr[j])) {
            tmp[k] = arr[i];
            i++;
        } else {
            tmp[k] = arr[j];
            j++;
        }
        k++;
    }
    while (i <= mid) tmp[k++] = arr[i++];
    while (j <= r) tmp[k++] = arr[j++];
    for (int i = l; i <= r; i++) {
        arr[i] = tmp[i];
    }
}

template <typename T>
void log(T x) {
    std::clog << "ATTENTION: " << x << std::endl;
}
}  // namespace utils
}  // namespace mua