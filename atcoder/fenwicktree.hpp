#ifndef ACL_PYTHON_FENWICKTREE
#define ACL_PYTHON_FENWICKTREE

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <structmember.h>
#include <algorithm>
#include <utility>
#include <vector>


#include "atcoder/utils"


namespace atcoder_python {

/* fenwick tree object **************************************/

/* Data structure that processes the following queries in O(logN) time.
(where N is length of given array.)
    * Updating an element
    * Calculating the sum of the elements of an interval


reference: https://github.com/atcoder/ac-library/blob/master/atcoder/fenwicktree.hpp

    ** Initialization **
    There are 2 ways to construct FenwickTree object:
    1. list size N

    >>> fw = FenwickTree(5)
    >>> fw
    FenwickTree([0, 0, 0, 0, 0])


    2. list a = [a_0, a_1, ..., a_{N - 1}]

    >>> fw = FenwickTree([0, 1, 2, 3, 4])
    >>> fw
    FenwickTree([0, 1, 2, 3, 4])



    ** output **
    builtin `print()` function outputs effective sequence instead of
    raw fenwicktree array.

    >>> fw = FenwickTree([0, 1, 2, 3, 4])
    >>> print(fw)
    FenwickTree([0, 1, 2, 3, 4])  # raw fenwicktree array: [0, 1, 2, 6, 4]



    ** update query **
    1. add x to a_p

    >>> fw = FenwickTree([0, 1, 2, 3, 4])
    >>> fw.add(2, 5)  # a_2 += 5
    >>> fw
    FenwickTree([0, 1, 7, 3, 4])


    2. update a_p with x

    >>> fw = FenwickTree([0, 1, 2, 3, 4])
    >>> fw.set(2, 7)  # a_2 = 7
    >>> fw
    FenwickTree([0, 1, 7, 3, 4])



    ** get query **
    1. get sum of values of interval [l, r) 

    >>> fw = FenwickTree([0, 1, 2, 3, 4])
    >>> fw.sum(1, 4)
    6                 # sum([a_1, a_2, a_3]) = sum([1, 2, 3])
    >>> fw.sum(0, 5)
    10
    >>> fw.sum(2, 2)
    0                 # interval [2, 2) has no element


    2. get a_p  (equivalent to fw.sum(p, p + 1))

    >>> fw = FenwickTree([0, 10, 20, 30, 40])
    >>> fw.get(1)
    10
    >>> fw.get(4)
    40

*/


struct FenwickTreeObject
{
    PyObject_HEAD
    int _n;
    std::vector<long long> data;
};

PyDoc_STRVAR(fenwicktree_doc,
"Given an array of length n, it processes the following \n"
"queries in \u039F(logn) time.\n"
"    * Updating an element\n"
"    * Calculating the sum of the elements of an interval\n\n"
"FenwickTree(n) (Constructor 1)\n"
"    Parameters\n"
"    ----------\n"
"    n : int\n"
"        length of array\n"
"    \n"
"    Returns\n"
"    -------\n"
"    fw : FenwickTree\n"
"        Zero initialized FenwickTree object of length n\n"
"    \n"
"    Constraints\n"
"    -----------\n"
"    0 <= n <= 10^7\n"
"    \n"
"    Complexity\n"
"    ----------\n"
"    \u039F(n)\n\n"
"FenwickTree(A) (Constructor 2)\n"
"    Parameters\n"
"    ----------\n"
"    A : list[int]\n"
"        array\n"
"    \n"
"    Returns\n"
"    -------\n"
"    fw : FenwickTree\n"
"        FenwickTree object initialized by array A\n"
"    \n"
"    Constraints\n"
"    -----------\n"
"    0 <= len(A) <= 10^7\n"
"    The sum of any interval of A fits into long long\n"
"    \n"
"    Complexity\n"
"    ----------\n"
"    \u039F(n)"
);


static Py_ssize_t
fenwicktree_length(FenwickTreeObject *self) {
    return (Py_ssize_t)self->_n;
}

static PySequenceMethods fenwicktree_as_sequence = {
    .sq_length = (lenfunc)fenwicktree_length,
};


static void
fenwicktree_add_impl(FenwickTreeObject *self, int p, long long x) {
    p++;
    while (p <= self->_n) {
        self->data[p - 1] += x;
        p += p & -p;
    }
    return;
}

static PyObject *
fenwicktree_add(FenwickTreeObject *self, PyObject *const *args, Py_ssize_t nargs) {
    if (nargs != 2) NARGS_VIOLATION("fenwicktree.add", 2);
    long p;
    int overflow = 0;
    long long x;
    p = PyLong_AsLong(args[0]);
    CHECK_CONVERT(p);
    CHECK_INDEX_RANGE(p, self->_n);

    x = PyLong_AsLongLongAndOverflow(args[1], &overflow);
    if (overflow) return NULL;
    CHECK_CONVERT(x);

    fenwicktree_add_impl(self, (int)p, x);
    Py_RETURN_NONE;
}

PyDoc_STRVAR(fenwicktree_add_doc,
"add(p, x)\n"
"--\n\n"
"It processes A[p] += x.\n\n"
"Parameters\n"
"----------\n"
"p : int\n"
"    index\n"
"x : int\n"
"    value\n"
"\n"
"Returns\n"
"-------\n"
"None : None\n"
"\n"
"Constraints\n"
"-----------\n"
"0 <= p < n\n"
"The sum of any interval of A fits into long long\n"
"\n"
"Complexity\n"
"----------\n"
"\u039F(logn)"
);

#define ACL_PYTHON_FENWICKTREE_ADD_METHODDEF \
    {"add", (PyCFunction)(void(*)(void))fenwicktree_add, METH_FASTCALL, fenwicktree_add_doc},


/* sum of [0, r) */
static long long
fenwicktree_sum_impl(FenwickTreeObject *self, int r) {
    long long s = 0;
    while (r > 0) {
        s += self->data[r - 1];
        r -= r & -r;
    }
    return s;
}

static PyObject *
fenwicktree_sum(FenwickTreeObject *self, PyObject *const *args, Py_ssize_t nargs) {
    if (nargs != 2) NARGS_VIOLATION("fenwicktree.sum", 2);
    long l, r;

    l = PyLong_AsLong(args[0]);
    CHECK_CONVERT(l);
    CHECK_INDEX_RANGE(l, self->_n + 1);

    r = PyLong_AsLong(args[1]);
    CHECK_CONVERT(r);
    CHECK_INDEX_RANGE(r, self->_n + 1);

    if (l > r) {
        PyErr_Format(PyExc_IndexError,
        "right index (%d) must be greater than or equal to left index (%d)", r, l);
        return NULL;
    }
    return PyLong_FromLongLong(fenwicktree_sum_impl(self, (int)r) - fenwicktree_sum_impl(self, (int)l));
}

PyDoc_STRVAR(fenwicktree_sum_doc,
"sum(l, r)\n"
"--\n\n"
"Calculate A[l] + A[l + 1] + ... + A[r - 1].\n\n"
"Parameters\n"
"----------\n"
"l : int\n"
"    left end of the interval\n"
"r : int\n"
"    right end of the interval\n"
"\n"
"Returns\n"
"-------\n"
"s : int\n"
"    sum of the interval [l, r) in A\n"
"\n"
"Constraints\n"
"-----------\n"
"0 <= l <= r <= n\n"
"\n"
"Complexity\n"
"----------\n"
"\u039F(logn)"
);

#define ACL_PYTHON_FENWICKTREE_SUM_METHODDEF \
    {"sum", (PyCFunction)(void(*)(void))fenwicktree_sum, METH_FASTCALL, fenwicktree_sum_doc},



static long long
fenwicktree_get_impl(FenwickTreeObject *self, int p) {
    return fenwicktree_sum_impl(self, p + 1) - fenwicktree_sum_impl(self, p);
}

static PyObject *
fenwicktree_get(FenwickTreeObject *self, PyObject *arg) {
    long p;
    p = PyLong_AsLong(arg);
    CHECK_CONVERT(p);
    CHECK_INDEX_RANGE(p, self->_n);
    return PyLong_FromLongLong(fenwicktree_get_impl(self, (int)p));
}

PyDoc_STRVAR(fenwicktree_get_doc,
"get(p)\n"
"--\n\n"
"It returns A[p].\n\n"
"Parameters\n"
"----------\n"
"p : int\n"
"    index\n"
"\n"
"Returns\n"
"-------\n"
"x : int\n"
"    A[p]\n"
"\n"
"Constraints\n"
"-----------\n"
"0 <= p < n\n"
"\n"
"Complexity\n"
"----------\n"
"\u039F(logn)"
);

#define ACL_PYTHON_FENWICKTREE_GET_METHODDEF \
    {"get", (PyCFunction)(void(*)(void))fenwicktree_get, METH_O, fenwicktree_get_doc},


static PyObject *
fenwicktree_set(FenwickTreeObject *self, PyObject *const *args, Py_ssize_t nargs) {
    if (nargs != 2) NARGS_VIOLATION("fenwicktree.set", 2);
    long p;
    int overflow = 0;
    long long x;
    p = PyLong_AsLong(args[0]);
    CHECK_CONVERT(p);
    CHECK_INDEX_RANGE(p, self->_n);

    x = PyLong_AsLongLongAndOverflow(args[1], &overflow);
    if (overflow) return NULL;
    CHECK_CONVERT(x);

    x -= fenwicktree_get_impl(self, (int)p);
    fenwicktree_add_impl(self, (int)p, x);
    Py_RETURN_NONE;   
}

PyDoc_STRVAR(fenwicktree_set_doc,
"set(p, x)\n"
"--\n\n"
"It processes A[p] = x.\n\n"
"Parameters\n"
"----------\n"
"p : int\n"
"    index\n"
"x : int\n"
"    value\n"
"\n"
"Returns\n"
"-------\n"
"None : None\n"
"\n"
"Constraints\n"
"-----------\n"
"0 <= p < n\n"
"The sum of any interval of A fits into long long\n"
"\n"
"Complexity\n"
"----------\n"
"\u039F(logn)"
);

#define ACL_PYTHON_FENWICKTREE_SET_METHODDEF \
    {"set", (PyCFunction)(void(*)(void))fenwicktree_set, METH_FASTCALL, fenwicktree_set_doc},


static PyObject *
fenwicktree_tolist_impl(FenwickTreeObject *self) {
    int n = self->_n;
    PyObject *list = PyList_New(n);
    long long l;
    long long r = 0;
    for (int i = 0; i < n; i++) {
        std::swap(l, r);
        r = fenwicktree_sum_impl(self, i + 1);
        PyList_SetItem(list, i, PyLong_FromLongLong(r - l));
    }
    return list;
}

static PyObject *
fenwicktree_tolist(FenwickTreeObject *self, PyObject *args) {
    return fenwicktree_tolist_impl(self);
}

PyDoc_STRVAR(fenwicktree_tolist_doc,
"tolist()\n"
"--\n\n"
"Conversion to list\n\n"
"Parameters\n"
"----------\n"
"Nothing\n"
"\n"
"Returns\n"
"-------\n"
"result : list[int]\n"
"\n"
"Constraints\n"
"-----------\n"
"Nothing\n"
"\n"
"Complexity\n"
"----------\n"
"\u039F(nlogn)"
);

#define ACL_PYTHON_FENWICKTREE_TOLIST_METHODDEF \
    {"tolist", (PyCFunction)fenwicktree_tolist, METH_NOARGS, fenwicktree_tolist_doc},


static PyMethodDef fenwicktree_methods[] = {
    ACL_PYTHON_FENWICKTREE_ADD_METHODDEF
    ACL_PYTHON_FENWICKTREE_SUM_METHODDEF
    ACL_PYTHON_FENWICKTREE_GET_METHODDEF
    ACL_PYTHON_FENWICKTREE_SET_METHODDEF
    ACL_PYTHON_FENWICKTREE_TOLIST_METHODDEF
    {NULL} /* Sentinel */
};

static PyObject *
fenwicktree_repr(FenwickTreeObject *self) {
    PyObject *list = fenwicktree_tolist_impl(self);
    PyObject *name = PyUnicode_FromString("FenwickTree");
    return sequential_object_repr((PyListObject *)list, name);
}




static int
fenwicktree_init(FenwickTreeObject *self, PyObject *args, PyObject *kwds) {
    static char *kwlist[] = {(char*)"n", NULL};
    PyObject *o;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O", kwlist, &o)) return -1;
    if (PyLong_Check(o)) {
        int n = (int)PyLong_AsLong(o);
        self->_n = n;
        self->data = std::vector<long long>(n);
        return 0;
    }
    if (PyList_Check(o)) {
        Py_ssize_t n = PyList_Size(o);
        self->_n = (int)n;
        self->data = std::vector<long long>(n);
        for (int i = 1; i <= n; i++) {
            PyObject *x = PyList_GetItem(o, (Py_ssize_t)(i - 1));
            if (!PyLong_Check(x)) {
                PyErr_SetString(PyExc_TypeError, "required: 'int' or 'list[int]'");
                return -1;
            }
            long long a = PyLong_AsLongLong(x);
            self->data[i - 1] += a;
            int j = i + (i & -i);
            if (j <= n) self->data[j - 1] += self->data[i - 1];
            // fenwicktree_add_impl(self, i - 1, a);
        }
        return 0;
    }
    PyErr_SetString(PyExc_TypeError, "required: 'int' or 'list[int]'");
    return -1;
}


static PyObject *
fenwicktree_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
    FenwickTreeObject *self;
    self = (FenwickTreeObject *)type->tp_alloc(type, 0);
    if (self == NULL) return NULL;
    return (PyObject *)self;
}

PyTypeObject FenwickTreeType = {
    .ob_base = {PyObject_HEAD_INIT(NULL) 0},
    .tp_name = "atcoder.FenwickTree",
    .tp_basicsize = sizeof(FenwickTreeObject),
    .tp_itemsize = 0,
    .tp_repr = (reprfunc)fenwicktree_repr,
    .tp_as_sequence = &fenwicktree_as_sequence,
    // .tp_getattro = PyObject_GenericGetAttr,
    // .tp_setattro = PyObject_GenericSetAttr,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_doc = fenwicktree_doc,
    .tp_methods = fenwicktree_methods,
    .tp_init = (initproc)fenwicktree_init,
    .tp_new = fenwicktree_new,
    .tp_free = PyObject_Del,
};

} // namespace atcoder_python


#endif  // ACL_PYTHON_FENWICKTREE