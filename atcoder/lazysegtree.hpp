#ifndef ACL_PYTHON_LAZYSEGTREE
#define ACL_PYTHON_LAZYSEGTREE



#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <structmember.h>
#include <algorithm>
#include <utility>
#include <vector>
#include <iostream>


#include "atcoder/utils"
#include "atcoder/lazysegtreeinfo"

namespace atcoder_python {
/* lazysegtree object *****************************************/

/*
reference: https://github.com/atcoder/ac-library/blob/master/document_en/lazysegtree.md
*/


extern PyTypeObject LazySegTreeType;






struct LazySegTreeObject
{
    PyObject_HEAD
    int _n, size, log;
    std::vector<lazyseginfo::S> d;
    std::vector<lazyseginfo::F> lz;
    PyObject *pyS;
    PyObject *pyF;

};



static void lazysegtree_update(LazySegTreeObject *self, int k);
static void lazysegtree_all_apply(LazySegTreeObject *self, int k, lazyseginfo::F f);
static void lazysegtree_push(LazySegTreeObject *self, int k);


static PyObject *
lazysegtree_repr(LazySegTreeObject *self) {
    // PyObject *list = _PyList_FromVector(self->d);
    PyObject *name = PyUnicode_FromString("LazySegTree");
    // return sequential_object_repr((PyListObject *)list, name);
    return name;
}

PyDoc_STRVAR(lazysegtree_doc,
"It is the data structure for monoids\n\n"
"SegTree(op, e, n, data=None)  (Constructor)\n"
"    Parameters\n"
"    ----------\n"
"    op : function\n"
"        binary operation\n"
"    e : function\n"
"        indentity element\n"
"    n_or_data : int (1) | list[int] (2)\n"
"        (1): array of length. All the elements are initialized to e()\n"
"        (2): initialized to given array\n"
"    \n"
"    Returns\n"
"    -------\n"
"    segtree : SegTree\n"
"        SegTree object\n"
"    \n"
"    Constraints\n"
"    -----------\n"
"    0 <= n <= 10^7\n"
"    \n"
"    Complexity\n"
"    ----------\n"
"    \u039F(n)"
);

static Py_ssize_t
lazysegtree_length(LazySegTreeObject *self) {
    return (Py_ssize_t)self->_n;
}

static PySequenceMethods lazysegtree_as_sequence = {
    .sq_length = (lenfunc)lazysegtree_length,
};



static void
lazysegtree_set_impl(LazySegTreeObject *self, int p, lazyseginfo::S s) {
    p += self->size;
    for (int i = self->log; i >= 1; i--) lazysegtree_push(self, p >> i);
    self->d[p] = s;
    for (int i = 1; i <= self->log; i++) lazysegtree_update(self, p >> i);
}


static PyObject *
lazysegtree_set(LazySegTreeObject *self, PyObject *const *args, Py_ssize_t nargs) {
    if (nargs != 2) NARGS_VIOLATION("LazySegTree.set", 2);
    int i;
    i = _PyObject_AsPositiveInt(args[0]);
    if (i == -1) CANNOT_CONVERT("LazySegTree.get", 0, "non-negative int");
    if (0 > i || i >= self->_n){
        PyErr_Format(PyExc_IndexError, "index %d is out of range", i);
        return NULL;
    }
    if (!PyObject_IsInstance(args[1], self->pyS)) {
        PyErr_SetString(PyExc_TypeError, "required type : S");
        return NULL;
    }
    lazyseginfo::S s = lazyseginfo::Sconvert_Py_to_C(args[1]);
    lazysegtree_set_impl(self, i, s);
    Py_RETURN_NONE;
}

PyDoc_STRVAR(lazysegtree_set_doc,
"set(i, s)\n"
"--\n\n"
"Set i-th element of the segtree to s.\n\n"
"Parameters\n"
"----------\n"
"i : int\n"
"    index\n"
"s : S\n"
"    new element\n"
"\n"
"Returns\n"
"-------\n"
"Nothing\n"
"\n"
"Constraints\n"
"-----------\n"
"0 <= i < n\n"
"\n"
"Complexity\n"
"----------\n"
"\u039F(log(n))"
);

#define ACL_PYTHON_LAZYSEGTREE_SET_METHODDEF \
    {"set", (PyCFunction)(void(*)(void))lazysegtree_set, METH_FASTCALL, lazysegtree_set_doc},


static lazyseginfo::S
lazysegtree_get_impl(LazySegTreeObject *self, int p) {
    for (int i = self->log; i >= 1; i--) lazysegtree_push(self, p >> i);
    return self->d[p + self->size];
}


static PyObject *
lazysegtree_get(LazySegTreeObject *self, PyObject *arg) {
    int i;
    i = _PyObject_AsPositiveInt(arg);
    if (i == -1) CANNOT_CONVERT("LazySegTree.get", 0, "non-negative int");
    if (0 > i || i >= self->_n){
        PyErr_Format(PyExc_IndexError, "index %d is out of range", i);
        return NULL;
    }
    lazyseginfo::S res = lazysegtree_get_impl(self, i);
    return lazyseginfo::Sconvert_C_to_Py(res, self->pyS);
}

PyDoc_STRVAR(lazysegtree_get_doc,
"get(i)\n"
"--\n\n"
"Get i-th element of the LazySegTree.\n\n"
"Parameters\n"
"----------\n"
"i : int\n"
"    index\n"
"\n"
"Returns\n"
"-------\n"
"element : S\n"
"    i-th element of the segtree\n"
"\n"
"Constraints\n"
"-----------\n"
"0 <= i < n\n"
"\n"
"Complexity\n"
"----------\n"
"\u039F(log(n))"
);

#define ACL_PYTHON_LAZYSEGTREE_GET_METHODDEF \
    {"get", (PyCFunction)(void(*)(void))lazysegtree_get, METH_O, lazysegtree_get_doc},



static lazyseginfo::S
lazysegtree_prod_impl(LazySegTreeObject *self, int l, int r) {
    if (l == r) return lazyseginfo::e();
    lazyseginfo::S sml = lazyseginfo::e(), smr = lazyseginfo::e();
    l += self->size;
    r += self->size;

    for (int i = self->log; i >= 1; i--) {
        if (((l >> i) << i) != l) lazysegtree_push(self, l >> i);
        if (((r >> i) << i) != r) lazysegtree_push(self, (r - 1) >> i);
    }

    while (l < r) {
            if (l & 1) sml = lazyseginfo::op(sml, self->d[l++]);
            if (r & 1) smr = lazyseginfo::op(self->d[--r], smr);
            l >>= 1;
            r >>= 1;
        }
    return lazyseginfo::op(sml, smr);
}


static PyObject *
lazysegtree_prod(LazySegTreeObject *self, PyObject *const *args, Py_ssize_t nargs) {
    if (nargs != 2) NARGS_VIOLATION("LazySegTree.prod", 2);
    int l, r;
    l = _PyObject_AsPositiveInt(args[0]);
    r = _PyObject_AsPositiveInt(args[1]);
    if (l == -1) CANNOT_CONVERT("LazySegTree.prod", 0, "non-negative int");
    if (r == -1) CANNOT_CONVERT("LazySegTree.prod", 1, "non-negative int");
    if (0 > l || l > self->_n){
        PyErr_Format(PyExc_IndexError, "index %d is out of range", l);
        return NULL;
    }
    if (0 > r || r > self->_n){
        PyErr_Format(PyExc_IndexError, "index %d is out of range", r);
        return NULL;
    }
    if (l > r) {
        PyErr_SetString(PyExc_IndexError, "required : l <= r");
        return NULL;
    }
    lazyseginfo::S res = lazysegtree_prod_impl(self, l, r);
    return lazyseginfo::Sconvert_C_to_Py(res, self->pyS);
}

PyDoc_STRVAR(lazysegtree_prod_doc,
"prod(l, r)\n"
"--\n\n"
"Get the product of [l, r).\n\n"
"Parameters\n"
"----------\n"
"l : int\n"
"    index\n"
"r : int\n"
"    index\n"
"\n"
"Returns\n"
"-------\n"
"prod : S\n"
"    product of [l, r)\n"
"\n"
"Constraints\n"
"-----------\n"
"0 <= l <= n\n"
"0 <= r <= n\n"
"l <= r\n"
"\n"
"Complexity\n"
"----------\n"
"\u039F(log(n))"
);

#define ACL_PYTHON_LAZYSEGTREE_PROD_METHODDEF \
    {"prod", (PyCFunction)(void(*)(void))lazysegtree_prod, METH_FASTCALL, lazysegtree_prod_doc},


static lazyseginfo::S
lazysegtree_all_prod_impl(LazySegTreeObject *self) {
    return self->d[1];
}

static PyObject *
lazysegtree_all_prod(LazySegTreeObject *self, PyObject *args) {
    return lazyseginfo::Sconvert_C_to_Py(lazysegtree_all_prod_impl(self), self->pyS);
}

PyDoc_STRVAR(lazysegtree_all_prod_doc,
"all_prod()\n"
"--\n\n"
"Get the all product.\n\n"
"Parameters\n"
"----------\n"
"Nothing\n"
"\n"
"Returns\n"
"-------\n"
"prod : S\n"
"    all product\n"
"\n"
"Constraints\n"
"-----------\n"
"Nothing\n"
"\n"
"Complexity\n"
"----------\n"
"\u039F(1)"
);

#define ACL_PYTHON_LAZYSEGTREE_ALL_PROD_METHODDEF \
    {"all_prod", (PyCFunction)lazysegtree_all_prod, METH_NOARGS, lazysegtree_all_prod_doc},



static void
lazysegtree_apply_at_impl(LazySegTreeObject *self, int p, lazyseginfo::F f) {
    p += self->size;
    for (int i = self->log; i >= 1; i--) lazysegtree_push(self, p >> i);
    self->d[p] = lazyseginfo::mapping(f, self->d[p]);
    for (int i = 1; i <= self->log; i++) lazysegtree_update(self, p >> i);
}


static PyObject *
lazysegtree_apply_at(LazySegTreeObject *self, PyObject *const *args, Py_ssize_t nargs) {
    if (nargs != 2) NARGS_VIOLATION("LazySegTree.apply_at", 2);
    int i = _PyObject_AsPositiveInt(args[0]);
    if (i == -1) CANNOT_CONVERT("LazySegTree.apply_at", 0, "non-negative int");
    if (0 > i || i >= self->_n){
        PyErr_Format(PyExc_IndexError, "index %d is out of range", i);
        return NULL;
    }
    if (!PyObject_IsInstance(args[1], self->pyF)) {
        PyErr_SetString(PyExc_TypeError, "required type : F");
        return NULL;
    }
    lazyseginfo::F f = lazyseginfo::Fconvert_Py_to_C(args[1]);
    lazysegtree_apply_at_impl(self, i, f);
    Py_RETURN_NONE;
}

PyDoc_STRVAR(lazysegtree_apply_at_doc,
"apply_at(i, f)\n"
"--\n\n"
"It applies a[i] = f(a[i]).\n\n"
"Parameters\n"
"----------\n"
"i : int\n"
"    index\n"
"f : F\n"
"    function\n"
"\n"
"Returns\n"
"-------\n"
"Nothing\n"
"\n"
"Constraints\n"
"-----------\n"
"0 <= i < n\n"
"\n"
"Complexity\n"
"----------\n"
"\u039F(log(n))"
);

#define ACL_PYTHON_LAZYSEGTREE_APPLY_AT_METHODDEF \
    {"apply_at", (PyCFunction)(void(*)(void))lazysegtree_apply_at, METH_FASTCALL, lazysegtree_apply_at_doc},


static void
lazysegtree_apply_impl(LazySegTreeObject *self, int l, int r, lazyseginfo::F f) {
    if (l == r) return;
    l += self->size;
    r += self->size;

    for (int i = self->log; i >= 1; i--) {
        if (((l >> i) << i) != l) lazysegtree_push(self, l >> i);
        if (((r >> i) << i) != r) lazysegtree_push(self, (r - 1) >> i);
    }

    {
        int l2 = l, r2 = r;
        while (l < r) {
            if (l & 1) lazysegtree_all_apply(self, l++, f);
            if (r & 1) lazysegtree_all_apply(self, --r, f);
            l >>= 1;
            r >>= 1;
        }
        l = l2;
        r = r2;
    }

    for (int i = 1; i <= self->log; i++) {
        if (((l >> i) << i) != l) lazysegtree_update(self, l >> i);
        if (((r >> i) << i) != r) lazysegtree_update(self, (r - 1) >> i);
    }
}

static PyObject *
lazysegtree_apply(LazySegTreeObject *self, PyObject *const *args, Py_ssize_t nargs) {
    if (nargs != 3) NARGS_VIOLATION("LazySegTree.apply", 2);
    int l, r;
    l = _PyObject_AsPositiveInt(args[0]);
    r = _PyObject_AsPositiveInt(args[1]);
    if (l == -1) CANNOT_CONVERT("LazySegTree.apply", 0, "non-negative int");
    if (r == -1) CANNOT_CONVERT("LazySegTree.apply", 1, "non-negative int");
    if (0 > l || l > self->_n){
        PyErr_Format(PyExc_IndexError, "index %d is out of range", l);
        return NULL;
    }
    if (0 > r || r > self->_n){
        PyErr_Format(PyExc_IndexError, "index %d is out of range", r);
        return NULL;
    }
    if (l > r) {
        PyErr_SetString(PyExc_IndexError, "required : l <= r");
        return NULL;
    }
    if (!PyObject_IsInstance(args[2], self->pyF)) {
        PyErr_SetString(PyExc_TypeError, "required type : F");
        return NULL;
    }
    lazyseginfo::F f = lazyseginfo::Fconvert_Py_to_C(args[2]);
    lazysegtree_apply_impl(self, l, r, f);
    Py_RETURN_NONE;
}

PyDoc_STRVAR(lazysegtree_apply_doc,
"apply(l, r, f)\n"
"--\n\n"
"It applies a[i] = f(a[i]) for all i in [l, r).\n\n"
"Parameters\n"
"----------\n"
"l : int\n"
"    index\n"
"r : int\n"
"    index\n"
"f : F\n"
"    function\n"
"\n"
"Returns\n"
"-------\n"
"Nothing\n"
"\n"
"Constraints\n"
"-----------\n"
"0 <= l <= n\n"
"0 <= r <= n\n"
"\n"
"Complexity\n"
"----------\n"
"\u039F(log(n))"
);

#define ACL_PYTHON_LAZYSEGTREE_APPLY_METHODDEF \
    {"apply", (PyCFunction)(void(*)(void))lazysegtree_apply, METH_FASTCALL, lazysegtree_apply_doc},


static PyMethodDef lazysegtree_methods[] = {
    ACL_PYTHON_LAZYSEGTREE_SET_METHODDEF
    ACL_PYTHON_LAZYSEGTREE_GET_METHODDEF
    ACL_PYTHON_LAZYSEGTREE_PROD_METHODDEF
    ACL_PYTHON_LAZYSEGTREE_ALL_PROD_METHODDEF
    ACL_PYTHON_LAZYSEGTREE_APPLY_AT_METHODDEF
    ACL_PYTHON_LAZYSEGTREE_APPLY_METHODDEF
    {NULL} /* Sentinel */
};


static void
lazysegtree_update(LazySegTreeObject *self, int k) {
    self->d[k] = lazyseginfo::op(self->d[2 * k], self->d[2 * k + 1]);
}

static void
lazysegtree_all_apply(LazySegTreeObject *self, int k, lazyseginfo::F f) {
    self->d[k] = lazyseginfo::mapping(f, self->d[k]);
    if (k < self->size) self->lz[k] = lazyseginfo::composition(f, self->lz[k]);
}

static void
lazysegtree_push(LazySegTreeObject *self, int k) {
    lazysegtree_all_apply(self, 2 * k, self->lz[k]);
    lazysegtree_all_apply(self, 2 * k + 1, self->lz[k]);
    self->lz[k] = lazyseginfo::id();
}


static int
lazysegtree_init(LazySegTreeObject *self, PyObject *args, PyObject *kwargs)
{
    PyObject *obj;
    if (!PyArg_ParseTuple(args, "O", &obj)) return -1;
    PyObject *pys = lazyseginfo::get_pys();
    Py_XINCREF(pys);
    Py_XDECREF(self->pyS);
    self->pyS = pys;
    PyObject *pyf = lazyseginfo::get_pyf();
    Py_XINCREF(pyf);
    Py_XDECREF(self->pyF);
    self->pyF = pyf;
    if (PyLong_Check(obj)) {
        int n = (int)PyLong_AsLong(obj);
        self->_n = n;
        int log = 0;
        while ((1 << log) < n) log++;
        self->log = log;
        int size = 1 << log;
        self->size = size;
        self->d = std::vector<lazyseginfo::S>(size << 1, lazyseginfo::e());
        self->lz = std::vector<lazyseginfo::F>(size, lazyseginfo::id());
    } else if (PyList_Check(obj)) {
        Py_ssize_t n = PyList_GET_SIZE(obj);
        self->_n = (int)n;
        int log = 0;
        while ((1 << log) < n) log++;
        self->log = log;
        int size = 1 << log;
        self->size = size;
        self->d = std::vector<lazyseginfo::S>(size << 1, lazyseginfo::e());
        self->lz = std::vector<lazyseginfo::F>(size, lazyseginfo::id());
        PyObject *item;
        for (Py_ssize_t i = 0; i < n; i++) {
            item = PyList_GetItem(obj, i);
            if (!PyObject_IsInstance(item, self->pyS)) {
                PyErr_SetString(PyExc_TypeError, "required: 'int' or 'list[S]'");
                return -1;
            }
            self->d[i + size] = lazyseginfo::Sconvert_Py_to_C(item);
        }
        for (int i = size - 1; i >= 1; i--) {
            lazysegtree_update(self, i);
        }
    } else {
        PyErr_SetString(PyExc_TypeError, "required: 'int' or 'list[S]'");
        return -1;
    }

    return 0;
}

static PyObject *
lazysegtree_new(PyTypeObject *type, PyObject *args, PyObject *kwargs)
{
    LazySegTreeObject *self;
    self = (LazySegTreeObject *)type->tp_alloc(type, 0);
    if (self != NULL) {
        self->_n = 0;
        self->d = std::vector<lazyseginfo::S>();
        self->pyS = Py_None;
        Py_INCREF(Py_None);
        self->pyF = Py_None;
        Py_INCREF(Py_None);
    }
    return (PyObject *)self;
}


static void
lazysegtree_dealloc(LazySegTreeObject *self) {
    Py_XDECREF(self->pyS);
    Py_XDECREF(self->pyF);
    Py_TYPE(self)->tp_free((PyObject*)self);
}


PyTypeObject LazySegTreeType = {
    .ob_base = {PyObject_HEAD_INIT(NULL) 0},
    .tp_name = "atcoder.LazySegTree",
    .tp_basicsize = sizeof(LazySegTreeObject),
    .tp_itemsize = 0,
    // .tp_itemsize = sizeof(int),
    .tp_dealloc = (destructor)lazysegtree_dealloc,
    .tp_repr = (reprfunc)lazysegtree_repr,
    .tp_as_sequence = &lazysegtree_as_sequence,
    // .tp_getattro = PyObject_GenericGetAttr,
    // .tp_setattro = PyObject_GenericSetAttr,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_doc = lazysegtree_doc,
    .tp_methods = lazysegtree_methods,
    // .tp_members = dsu_members,
    .tp_init = (initproc)lazysegtree_init,
    .tp_new = lazysegtree_new,
    .tp_free = PyObject_Del,
};


} // namespace atcoder_python

#endif  // ACL_PYTHON_LAZYSEGTREE