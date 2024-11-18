#ifndef ACL_PYTHON_SEGTREEINFO
#define ACL_PYTHON_SEGTREEINFO

#include <Python.h>
#include <structmember.h>
#include <iostream>
#include <algorithm>

#include "internal_modint.hpp"
#include "modint.hpp"

namespace atcoder_python {

namespace seginfo {

using S = static_modint<998244353>;
using mint = static_modint<998244353>;

S op(S a, S b) {
    return a + b;
}

S e() {
    return 0;
}

static PyObject *
get_pys() {
    return (PyObject *)&ModIntType;
}

static PyObject *
Sconvert_C_to_Py(const S& s, PyObject *pys) {
    PyObject *res = (PyObject *)ModInt_FromUnsignedInt(s.val());
    return res;
}

static S
Sconvert_Py_to_C(PyObject *pys) {
    mint s(ModInt_AsUnsignedInt(pys));
    return s;
}

} // namespace seginfo

} // namespace atcoder_python

#endif  // ACL_PYTHON_SEGTREEINFO
