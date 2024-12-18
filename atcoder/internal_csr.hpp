#ifndef ACL_PYTHON_INTERNAL_CSR
#define ACL_PYTHON_INTERNAL_CSR

#include <algorithm>
#include <utility>
#include <vector>



/*
Function to convert an edge set to CSR format

reference: https://github.com/atcoder/ac-library/blob/master/atcoder/internal_scc.hpp
*/



namespace atcoder_python {
namespace internal {

template <class E> struct csr {
    std::vector<int> start;
    std::vector<E> elist;
    explicit csr(int n, const std::vector<std::pair<int, E>>& edges)
        : start(n + 1), elist(edges.size()) {
        for (auto e : edges) {
            start[e.first + 1]++;
        }
        for (int i = 1; i <= n; i++) {
            start[i] += start[i - 1];
        }
        auto counter = start;
        for (auto e : edges) {
            elist[counter[e.first]++] = e.second;
        }
    }
};

}  // namespace internal

}  // namespace atcoder_python

#endif  // ACL_PYTHON_INTERNAL_CSR