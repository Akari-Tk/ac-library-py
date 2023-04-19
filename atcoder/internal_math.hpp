#ifndef ACL_PYTHON_INTERNAL_MATH
#define ACL_PYTHON_INTERNAL_MATH

#include <algorithm>
#include <utility>
#include <tuple>
#include <vector>


// reference: https://github.com/atcoder/ac-library/blob/master/atcoder/internal_math.hpp



namespace atcoder_python {



constexpr long long
safe_mod(long long x, long long m) {
    x %= m;
    if (x < 0) x += m;
    return x;
}


constexpr std::pair<long long, long long>
inv_gcd(long long a, long long b)
{
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


std::pair<long long, long long>
_crt(const std::vector<long long>& r, const std::vector<long long>& m) {
    int n = int(r.size());
    // Contracts: 0 <= r0 < m0
    long long r0 = 0, m0 = 1;
    for (int i = 0; i < n; i++) {
        long long r1 = safe_mod(r[i], m[i]), m1 = m[i];
        if (m0 < m1) {
            std::swap(r0, r1);
            std::swap(m0, m1);
        }
        if (m0 % m1 == 0) {
            if (r0 % m1 != r1) return {0, 0};
            continue;
        }
        // assume: m0 > m1, lcm(m0, m1) >= 2 * max(m0, m1)

        // (r0, m0), (r1, m1) -> (r2, m2 = lcm(m0, m1));
        // r2 % m0 = r0
        // r2 % m1 = r1
        // -> (r0 + x*m0) % m1 = r1
        // -> x*u0*g = r1-r0 (mod u1*g) (u0*g = m0, u1*g = m1)
        // -> x = (r1 - r0) / g * inv(u0) (mod u1)

        // im = inv(u0) (mod u1) (0 <= im < u1)
        long long g, im;
        std::tie(g, im) = inv_gcd(m0, m1);

        long long u1 = (m1 / g);
        // |r1 - r0| < (m0 + m1) <= lcm(m0, m1)
        if ((r1 - r0) % g) return {0, 0};

        // u1 * u1 <= m1 * m1 / g / g <= m0 * m1 / g = lcm(m0, m1)
        long long x = (r1 - r0) / g % u1 * im % u1;

        // |r0| + |m0 * x|
        // < m0 + m0 * (u1 - 1)
        // = m0 + m0 * m1 / g - m0
        // = lcm(m0, m1)
        r0 += x * m0;
        m0 *= u1;  // -> lcm(m0, m1)
        if (r0 < 0) r0 += m0;
    }
    return {r0, m0};
}


constexpr long long
pow_mod_constexpr(long long x, long long n, int m) {
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

constexpr bool
is_prime_constexpr(int n) {
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



unsigned long long
floor_sum_unsigned(unsigned long long n,
                   unsigned long long m,
                   unsigned long long a,
                   unsigned long long b) {
    unsigned long long ans = 0;
    while (true) {
        if (a >= m) {
            ans += n * (n - 1) / 2 * (a / m);
            a %= m;
        }
        if (b >= m) {
            ans += n * (b / m);
            b %= m;
        }

        unsigned long long y_max = a * n + b;
        if (y_max < m) break;
        // y_max < m * (n + 1)
        // floor(y_max / m) <= n
        n = (unsigned long long)(y_max / m);
        b = (unsigned long long)(y_max % m);
        std::swap(m, a);
    }
    return ans;
}


} // namespace atcoder_python


#endif  // ACL_PYTHON_INTERNAL_MATH