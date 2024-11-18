#ifndef ACL_PYTHON_LAZYSEGTREEINFO
#define ACL_PYTHON_LAZYSEGTREEINFO

#include <Python.h>
#include <structmember.h>
#include <iostream>
#include <algorithm>

#include "internal_modint.hpp"
#include "modint.hpp"

namespace atcoder_python {

namespace lazyseginfo {

using mint = static_modint<998244353>;

struct S {
    mint a;
    mint length;
    S() : a(0), length(0) {}
    S(mint a, mint length) : a(a), length(length) {}
};

S op(S a, S b) {
    return S(a.a + b.a, a.length + b.length);
}

S e() {
    return S();
}

struct F {
    mint b;
    mint c;
    F() : b(1), c(0) {}
    F(mint b, mint c) : b(b), c(c) {}
};

S mapping(F f, S x) {
    return S(f.b * x.a + f.c * x.length, x.length);
}

F composition(F f, F g) {
    return F(f.b * g.b, f.b * g.c + f.c);
}

F id() {
    return F();
}

static PyObject *
get_pys() {
    // Import the Python module
    PyObject *pName = PyUnicode_DecodeFSDefault("__main__");
    PyObject *pModule = PyImport_Import(pName);
    Py_DECREF(pName);
    if (pModule != nullptr) {
        // Get the T class from the module
        PyObject *pClass = PyObject_GetAttrString(pModule, "S");
        if (pClass && PyCallable_Check(pClass)) {
            Py_DECREF(pModule);
            return pClass;
        } else {
            PyErr_Print();
            Py_XDECREF(pClass);
            Py_DECREF(pModule);
            std::cerr << "class not found" << std::endl;
        }
    }
    return nullptr;
}

static PyObject *
Sconvert_C_to_Py(const S& s, PyObject *pys) {
    PyObject *args = PyTuple_New(2);
    PyTuple_SetItem(args, 0, (PyObject *)ModInt_FromUnsignedInt(s.a.val()));
    PyTuple_SetItem(args, 1, (PyObject *)ModInt_FromUnsignedInt(s.length.val()));
    PyObject *res = PyObject_CallObject(pys, args);
    Py_DECREF(args);
    return res;
}

static unsigned int
convert_to_UnsignedInt(PyObject *obj) {
    if (PyLong_Check(obj)) return (unsigned int)PyLong_AsUnsignedLong(obj);
    if (ModInt_Check(obj)) return ModInt_AsUnsignedInt(obj);
    return 0;
}

static S
Sconvert_Py_to_C(PyObject *pys) {
    S s;
    PyObject *a = PyObject_GetAttrString(pys, "a");
    s.a = mint(convert_to_UnsignedInt(a));
    Py_DECREF(a);
    PyObject *length = PyObject_GetAttrString(pys, "length");
    s.length = mint(convert_to_UnsignedInt(length));
    Py_DECREF(length);
    return s;
}

static PyObject *
get_pyf() {
    // Import the Python module
    PyObject *pName = PyUnicode_DecodeFSDefault("__main__");
    PyObject *pModule = PyImport_Import(pName);
    Py_DECREF(pName);
    if (pModule != nullptr) {
        // Get the T class from the module
        PyObject *pClass = PyObject_GetAttrString(pModule, "F");
        if (pClass && PyCallable_Check(pClass)) {
            Py_DECREF(pModule);
            return pClass;
        } else {
            PyErr_Print();
            Py_XDECREF(pClass);
            Py_DECREF(pModule);
            std::cerr << "class not found" << std::endl;
        }
    }
    return nullptr;
}

static F
Fconvert_Py_to_C(PyObject *pyf) {
    F f;
    PyObject *b = PyObject_GetAttrString(pyf, "b");
    f.b = mint(convert_to_UnsignedInt(b));
    Py_DECREF(b);
    PyObject *c = PyObject_GetAttrString(pyf, "c");
    f.c = mint(convert_to_UnsignedInt(c));
    Py_DECREF(c);
    return f;
}

} // namespace lazyseginfo

} // namespace atcoder_python

#endif  // ACL_PYTHON_LAZYSEGTREEINFO
