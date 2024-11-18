#ifndef ACL_PYTHON_MATH
#define ACL_PYTHON_MATH


#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <utility>
#include <vector>

#include "atcoder/utils"
#include "atcoder/internal_math"


// reference: https://github.com/atcoder/ac-library/blob/master/atcoder/math.hpp
// reference: https://github.com/atcoder/ac-library/blob/master/atcoder/internal_math.hpp





namespace atcoder_python {


/* inv_mod  ************************************************/

/* calculate inverse element of x in mod m.

    >>> m = 7
    >>> inv_mod(3, m)
    5                   # 3 * 5 == 1  (mod 7)
    >>> inv_mod(7, m)
    ValueError          # gcd(7, 7) > 1.



*/

static PyObject *
inv_mod(PyObject *module, PyObject *const *args, Py_ssize_t nargs) {
    const char* name = "inv_mod";
    long long x = 0, m = 1;
    int overflow;
    if (nargs != 2) NARGS_VIOLATION(name, 2);
    CONVERT_LONG_LONG(x, 0, overflow, name);
    CONVERT_LONG_LONG(m, 1, overflow, name);
    // assert(1 <= m);
    if (m < 1) {
        PyErr_SetString(PyExc_ValueError, "m must be out greater than or equal to 1");
        return NULL;
    }
    auto z = inv_gcd(x, m);
    // assert(z.first == 1);
    if (z.first != 1) {
        PyErr_Format(PyExc_ValueError,
                     "There is no inverse element of %lld in mod %lld", x, m);
        return NULL;
    }
    return PyLong_FromLongLong(z.second);
}

PyDoc_STRVAR(inv_mod_doc,
"inv_mod($module, x, m)\n"
"--\n\n"
"Returns integer y s.t. 0 <= y < m and x * y == 1 (mod m)\n\n"
"Parameters\n"
"----------\n"
"x : int\n"
"    integer\n"
"m : int\n"
"    modulus\n"
"\n"
"Returns\n"
"-------\n"
"y : int\n"
"    modular multiplicative inverse of x\n"
"\n"
"Constraints\n"
"-----------\n"
"-2^63 <= x < 2^63\n"
"1 <= m < 2^63\n"
"gcd(x, m) = 1\n"
"\n"
"Complexity\n"
"----------\n"
"\u039F(logm)"
);

#define ACL_PYTHON_INV_MOD_METHODDEF \
    {"inv_mod", (PyCFunction)(void(*)(void))inv_mod, METH_FASTCALL, inv_mod_doc},


/*  crt ****************************************************/

/* Given two arrays r, m with length n, it solves the modular equation system
    x == r[i] (mod m[i]) for all i = 1 ... n


    Ex.)
        >>> r = [3, 4]
        >>> m = [5, 7]

        -> Solve　the modular equation system:
            * x == 3 (mod 5)
            * x == 4 (mod 7)

        >>> z = crt(r, m)
        >>> z
        (18, 35)           # 35 * n + 18 == 3 (mod 5) && 35 * n + 18 == 4 (mod 7)\
                             for all integer n


*/


static PyObject *
crt(PyObject *module, PyObject *args, PyObject *kwargs) {
    static char *kwlist[] = {(char *)"r", (char *)"m", NULL};
    PyObject *r_obj;
    PyObject *m_obj;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "OO", kwlist, &r_obj, &m_obj)) return NULL;
    if (!PyList_Check(r_obj) || !PyList_Check(r_obj)) {
        PyErr_SetString(PyExc_TypeError, "required : list[int], list[int]");
        return NULL;
    }
    int err = 0;
    auto r = _PyList_AsVectorLongLong(r_obj, &err);
    if (err) return NULL;
    err = 0;
    auto m = _PyList_AsVectorLongLong(m_obj, &err);
    if (err) return NULL;
    if (r.size() != m.size()) {
        PyErr_SetString(PyExc_ValueError, "both arrays need to have same length");
        return NULL;
    }
    for (int i = 0; i < (int)m.size(); i++) {
        if (m[i] < 1) {
            PyErr_SetString(PyExc_ValueError, "m[i] >= 1 is required");
            return NULL;
        }
    }
    std::pair<long long, long long> z = _crt(r, m);
    return Py_BuildValue("(LL)", z.first, z.second);
}

PyDoc_STRVAR(crt_doc,
"crt($module, r, m)\n"
"--\n\n"
"Given two lists r, m with length n, it solves the modular equation system:\n"
"    x == r[i] (mod m[i]) forall i in {0, ..., n-1}\n\n"
"If there is no solution, it returns (0, 0).\n"
"Otherwise, all the solutions can be written as the form\n"
"x == y (mod z), using integers y, z (0 <= y < z = lcm(m[i]))\n"
"It returns this (y, z) as tuple.\n"
"If n == 0, it returns (0, 1).\n\n"
"Parameters\n"
"----------\n"
"r : list[int]\n"
"    array of integers\n"
"m : list[int]\n"
"    array of moduli\n"
"\n"
"Returns\n"
"-------\n"
"result : tuple[int, int]\n"
"    solutions of given modular equation system\n"
"\n"
"Constraints\n"
"-----------\n"
"len(r) == len(m)\n"
"-2^63 <= r[i] < 2^63\n"
"1 <= m[i] < 2^63\n"
"lcm(m[i]) is in long long\n"
"\n"
"Complexity\n"
"----------\n"
"\u039F(nloglcm(m[i]))"
);

#define ACL_PYTHON_CRT_METHODDEF \
    {"crt", (PyCFunction)crt, METH_VARARGS | METH_KEYWORDS, crt_doc},



static PyObject *
floor_sum(PyObject *module, PyObject *const *args, Py_ssize_t nargs) {
    const char *name = "floor_sum";
    long long n = 0, m = 0, a = 0, b = 0;
    if (nargs != 4) NARGS_VIOLATION(name, 4);
    int overflow;
    CONVERT_LONG_LONG(n, 0, overflow, name);
    CONVERT_LONG_LONG(m, 1, overflow, name);
    CONVERT_LONG_LONG(a, 2, overflow, name);
    CONVERT_LONG_LONG(b, 3, overflow, name);
    // assert(0 <= n && n < (1LL << 32));
    if (0 > n || n >= (1LL << 32)){
        PyErr_SetString(PyExc_ValueError, "[constraints] 0 <= n < 2^32");
        return NULL;
    }
    // assert(1 <= m && m < (1LL << 32));
    if (1 > m || m >= (1LL << 32)){
        PyErr_SetString(PyExc_ValueError, "[constraints] 1 <= m < 2^32");
        return NULL;
    }
    unsigned long long ans = 0;
    if (a < 0) {
        unsigned long long a2 = safe_mod(a, m);
        ans -= 1ULL * n * (n - 1) / 2 * ((a2 - a) / m);
        a = a2;
    }
    if (b < 0) {
        unsigned long long b2 = safe_mod(b, m);
        ans -= 1ULL * n * ((b2 - b) / m);
        b = b2;
    }
    ans += floor_sum_unsigned(n, m, a, b);
    return PyLong_FromUnsignedLongLong(ans);
}

PyDoc_STRVAR(floor_sum_doc,
"floor_sum($module, n, m, a, b)\n"
"--\n\n"
"Calculate sum((a * i + b) // m for i in range(n))\n"
"It returns the answer in mod 2^64, if overflowed\n\n"
"Parameters\n"
"----------\n"
"n : int\n"
"    integer\n"
"m : int\n"
"    integer\n"
"a : int\n"
"    integer\n"
"b : int\n"
"    integer\n"
"\n"
"Returns\n"
"-------\n"
"result : int\n"
"    the answer of floor sum\n"
"\n"
"Constraints\n"
"-----------\n"
"0 <= n < 2^32\n"
"1 <= m < 2^32\n"
"-2^63 <= a < 2^63\n"
"-2^63 <= b < 2^63\n"
"\n"
"Complexity\n"
"----------\n"
"\u039F(logm)"
);

#define ACL_PYTHON_FLOOR_SUM_METHODDEF \
    {"floor_sum", (PyCFunction)(void(*)(void))floor_sum, METH_FASTCALL, floor_sum_doc},

static PyObject *
is_prime(PyObject *module, PyObject *arg) {
    if (!PyLong_Check(arg)) {
        PyErr_SetString(PyExc_TypeError, "is_prime() takes int");
        return NULL;
    }
    long n = PyLong_AsLong(arg);
    if (n < 0 || n >= (1L << 31)) CANNOT_CONVERT("is_prime", 0, "non-negative int");
    if (is_prime_constexpr((int)n)) Py_RETURN_TRUE;
    Py_RETURN_FALSE;
}

PyDoc_STRVAR(is_prime_doc,
"is_prime($module, n)\n"
"--\n\n"
"Miller-Rabin primality test for integer n\n\n"
"Parameters\n"
"----------\n"
"n: int\n"
"    the number tested for primality\n"
"\n"
"Returns\n"
"-------\n"
"result : bool\n"
"    whether n is prime or not\n"
"\n"
"Constraints\n"
"-----------\n"
"0 <= n < 2^31\n"
"\n"
"Complexity\n"
"----------\n"
"TODO"
);

#define ACL_PYTHON_IS_PRIME_METHODDEF \
    {"is_prime", (PyCFunction)is_prime, METH_O, is_prime_doc},


PyMethodDef mathfunctions[] = {
    ACL_PYTHON_INV_MOD_METHODDEF
    ACL_PYTHON_CRT_METHODDEF
    ACL_PYTHON_FLOOR_SUM_METHODDEF
    ACL_PYTHON_IS_PRIME_METHODDEF
    {NULL} /* Sentinel */
};


} // mamespace atcoder_python

#endif  // ACL_PYTHON_MATH