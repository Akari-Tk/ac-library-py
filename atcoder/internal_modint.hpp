#ifndef ACL_PYTHON_INTERNAL_MODINT
#define ACL_PYTHON_INTERNAL_MODINT



#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <structmember.h>
#include <algorithm>
#include <utility>
#include <type_traits>



namespace atcoder_python {


namespace internal {

constexpr long long safe_mod(long long x, long long m) {
    x %= m;
    if (x < 0) x += m;
    return x;
}

constexpr long long pow_mod_constexpr(long long x, long long n, int m) {
    if (m == 1) return 0;
    unsigned int _m = (unsigned int)(m);
    unsigned long long r = 1;
    unsigned long long y = safe_mod(x, m);
    while (n) {
        if (n & 1) r = (r * y) % _m;
        y = (y * y) % _m;
        n >>= 1;
    }
    return r;
}

constexpr bool is_prime_constexpr(int n) {
    if (n <= 1) return false;
    if (n == 2 || n == 7 || n == 61) return true;
    if (n % 2 == 0) return false;
    long long d = n - 1;
    while (d % 2 == 0) d /= 2;
    constexpr long long bases[3] = {2, 7, 61};
    for (long long a : bases) {
        long long t = d;
        long long y = pow_mod_constexpr(a, t, n);
        while (t != n - 1 && y != 1 && y != n - 1) {
            y = y * y % n;
            t <<= 1;
        }
        if (y != n - 1 && t % 2 == 0) {
            return false;
        }
    }
    return true;
}
template <int n> constexpr bool is_prime = is_prime_constexpr(n);

constexpr std::pair<long long, long long> inv_gcd(long long a, long long b) {
    a = safe_mod(a, b);
    if (a == 0) return {b, 0};
    long long s = b, t = a;
    long long m0 = 0, m1 = 1;
    while (t) {
        long long u = s / t;
        s -= t * u;
        m0 -= m1 * u;
        auto tmp = s;
        s = t;
        t = tmp;
        tmp = m0;
        m0 = m1;
        m1 = tmp;
    }
    if (m0 < 0) m0 += b / s;
    return {s, m0};
}


} // namespace internal




template <int m, std::enable_if_t<(1 <= m)>* = nullptr>
struct static_modint {
    using _mint = static_modint;

  public:
    static constexpr int mod() { return m; }
    static _mint raw(int v) {
        _mint x;
        x._v = v;
        return x;
    }

    static_modint() : _v(0) {}
    static_modint(unsigned int v) {
        _v = (unsigned int)(v % umod());
    }

    unsigned int val() const { return _v; }

    _mint& operator++() {
        _v++;
        if (_v == umod()) _v = 0;
        return *this;
    }
    _mint& operator--() {
        if (_v == 0) _v = umod();
        _v--;
        return *this;
    }
    _mint operator++(int) {
        _mint result = *this;
        ++*this;
        return result;
    }
    _mint operator--(int) {
        _mint result = *this;
        --*this;
        return result;
    }

    _mint& operator+=(const _mint& rhs) {
        _v += rhs._v;
        if (_v >= umod()) _v -= umod();
        return *this;
    }
    _mint& operator-=(const _mint& rhs) {
        _v -= rhs._v;
        if (_v >= umod()) _v += umod();
        return *this;
    }
    _mint& operator*=(const _mint& rhs) {
        unsigned long long z = _v;
        z *= rhs._v;
        _v = (unsigned int)(z % umod());
        return *this;
    }
    _mint& operator/=(const _mint& rhs) { return *this = *this * rhs.inv(); }

    _mint operator+() const { return *this; }
    _mint operator-() const { return _mint() - *this; }

    _mint pow(long long n) const {
        assert(0 <= n);
        _mint x = *this, r = 1;
        while (n) {
            if (n & 1) r *= x;
            x *= x;
            n >>= 1;
        }
        return r;
    }
    _mint inv() const {
        if (prime) {
            assert(_v);
            return pow(umod() - 2);
        } else {
            auto eg = internal::inv_gcd(_v, m);
            assert(eg.first == 1);
            return eg.second;
        }
    }

    friend _mint operator+(const _mint& lhs, const _mint& rhs) {
        return _mint(lhs) += rhs;
    }
    friend _mint operator-(const _mint& lhs, const _mint& rhs) {
        return _mint(lhs) -= rhs;
    }
    friend _mint operator*(const _mint& lhs, const _mint& rhs) {
        return _mint(lhs) *= rhs;
    }
    friend _mint operator/(const _mint& lhs, const _mint& rhs) {
        return _mint(lhs) /= rhs;
    }
    friend bool operator==(const _mint& lhs, const _mint& rhs) {
        return lhs._v == rhs._v;
    }
    friend bool operator!=(const _mint& lhs, const _mint& rhs) {
        return lhs._v != rhs._v;
    }

  private:
    unsigned int _v;
    static constexpr unsigned int umod() { return m; }
    static constexpr bool prime = internal::is_prime<m>;
};


} // namespace atcoder_python

#endif  // ACL_PYTHON_INTERNAL_MODINT