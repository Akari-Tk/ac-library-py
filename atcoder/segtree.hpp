#ifndef ACL_PYTHON_SEGTREE
#define ACL_PYTHON_SEGTREE



#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <structmember.h>
#include <algorithm>
#include <utility>
#include <vector>
#include <iostream>


#include "atcoder/utils"
#include "atcoder/segtreeinfo"

namespace atcoder_python {
/* segtree object *****************************************/

/*
reference: https://github.com/atcoder/ac-library/blob/master/document_en/segtree.md
*/


extern PyTypeObject SegTreeType;






struct SegTreeObject
{
    PyObject_HEAD
    int _n, size, log;
    std::vector<seginfo::S> d;
    PyObject *pyS;

};



static void segtree_update(SegTreeObject *self, int k);


static PyObject *
segtree_repr(SegTreeObject *self) {
    // PyObject *list = _PyList_FromVector(self->d);
    PyObject *name = PyUnicode_FromString("SegTree");
    // return sequential_object_repr((PyListObject *)list, name);
    return name;
}

PyDoc_STRVAR(segtree_doc,
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
segtree_length(SegTreeObject *self) {
    return (Py_ssize_t)self->_n;
}

static PySequenceMethods segtree_as_sequence = {
    .sq_length = (lenfunc)segtree_length,
};



static void
segtree_set_impl(SegTreeObject *self, int p, seginfo::S s) {
    p += self->size;
    self->d[p] = s;
    for (int i = 1; i <= self->log; i++) segtree_update(self, p >> i);
}


static PyObject *
segtree_set(SegTreeObject *self, PyObject *const *args, Py_ssize_t nargs) {
    if (nargs != 2) NARGS_VIOLATION("SegTree.set", 2);
    int i;
    i = _PyObject_AsPositiveInt(args[0]);
    if (i == -1) CANNOT_CONVERT("SegTree.get", 0, "non-negative int");
    if (0 > i || i >= self->_n){
        PyErr_Format(PyExc_IndexError, "index %d is out of range", i);
        return NULL;
    }
    if (!PyObject_IsInstance(args[1], self->pyS)) {
        PyErr_SetString(PyExc_TypeError, "required type : S");
        return NULL;
    }
    seginfo::S s = seginfo::Sconvert_Py_to_C(args[1]);
    segtree_set_impl(self, i, s);
    Py_RETURN_NONE;
}

PyDoc_STRVAR(segtree_set_doc,
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

#define ACL_PYTHON_SEGTREE_SET_METHODDEF \
    {"set", (PyCFunction)(void(*)(void))segtree_set, METH_FASTCALL, segtree_set_doc},


static seginfo::S
segtree_get_impl(SegTreeObject *self, int i) {
    return self->d[i + self->size];
}


static PyObject *
segtree_get(SegTreeObject *self, PyObject *arg) {
    int i;
    i = _PyObject_AsPositiveInt(arg);
    if (i == -1) CANNOT_CONVERT("SegTree.get", 0, "non-negative int");
    if (0 > i || i >= self->_n){
        PyErr_Format(PyExc_IndexError, "index %d is out of range", i);
        return NULL;
    }
    seginfo::S res = segtree_get_impl(self, i);
    return seginfo::Sconvert_C_to_Py(res, self->pyS);
}

PyDoc_STRVAR(segtree_get_doc,
"get(i)\n"
"--\n\n"
"Get i-th element of the segtree.\n\n"
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
"\u039F(1)"
);

#define ACL_PYTHON_SEGTREE_GET_METHODDEF \
    {"get", (PyCFunction)(void(*)(void))segtree_get, METH_O, segtree_get_doc},



static seginfo::S
segtree_prod_impl(SegTreeObject *self, int l, int r) {
    seginfo::S sml = seginfo::e(), smr = seginfo::e();
    l += self->size;
    r += self->size;
    while (l < r) {
            if (l & 1) sml = seginfo::op(sml, self->d[l++]);
            if (r & 1) smr = seginfo::op(self->d[--r], smr);
            l >>= 1;
            r >>= 1;
        }
    return seginfo::op(sml, smr);
}


static PyObject *
segtree_prod(SegTreeObject *self, PyObject *const *args, Py_ssize_t nargs) {
    if (nargs != 2) NARGS_VIOLATION("SegTree.prod", 2);
    int l, r;
    l = _PyObject_AsPositiveInt(args[0]);
    r = _PyObject_AsPositiveInt(args[1]);
    if (l == -1) CANNOT_CONVERT("SegTree.prod", 0, "non-negative int");
    if (r == -1) CANNOT_CONVERT("SegTree.prod", 1, "non-negative int");
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
    seginfo::S res = segtree_prod_impl(self, l, r);
    return seginfo::Sconvert_C_to_Py(res, self->pyS);
}

PyDoc_STRVAR(segtree_prod_doc,
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

#define ACL_PYTHON_SEGTREE_PROD_METHODDEF \
    {"prod", (PyCFunction)(void(*)(void))segtree_prod, METH_FASTCALL, segtree_prod_doc},


static seginfo::S
segtree_all_prod_impl(SegTreeObject *self) {
    return self->d[1];
}

static PyObject *
segtree_all_prod(SegTreeObject *self, PyObject *args) {
    return seginfo::Sconvert_C_to_Py(segtree_all_prod_impl(self), self->pyS);
}

PyDoc_STRVAR(segtree_all_prod_doc,
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

#define ACL_PYTHON_SEGTREE_ALL_PROD_METHODDEF \
    {"all_prod", (PyCFunction)segtree_all_prod, METH_NOARGS, segtree_all_prod_doc},


static PyMethodDef segtree_methods[] = {
    ACL_PYTHON_SEGTREE_SET_METHODDEF
    ACL_PYTHON_SEGTREE_GET_METHODDEF
    ACL_PYTHON_SEGTREE_PROD_METHODDEF
    ACL_PYTHON_SEGTREE_ALL_PROD_METHODDEF
    {NULL} /* Sentinel */
};


static void
segtree_update(SegTreeObject *self, int k) {
    self->d[k] = seginfo::op(self->d[2 * k], self->d[2 * k + 1]);
}


static int
segtree_init(SegTreeObject *self, PyObject *args, PyObject *kwargs)
{
    PyObject *obj;
    if (!PyArg_ParseTuple(args, "O", &obj)) return -1;
    PyObject *pys = seginfo::get_pys();
    Py_XINCREF(pys);
    Py_XDECREF(self->pyS);
    self->pyS = pys;
    if (PyLong_Check(obj)) {
        int n = (int)PyLong_AsLong(obj);
        self->_n = n;
        int log = 0;
        while ((1 << log) < n) log++;
        self->log = log;
        int size = 1 << log;
        self->size = size;
        self->d = std::vector<seginfo::S>(size << 1, seginfo::e());
    } else if (PyList_Check(obj)) {
        Py_ssize_t n = PyList_GET_SIZE(obj);
        self->_n = (int)n;
        int log = 0;
        while ((1 << log) < n) log++;
        self->log = log;
        int size = 1 << log;
        self->size = size;
        self->d = std::vector<seginfo::S>(size << 1, seginfo::e());
        PyObject *item;
        for (Py_ssize_t i = 0; i < n; i++) {
            item = PyList_GetItem(obj, i);
            if (!PyObject_IsInstance(item, self->pyS)) {
                PyErr_SetString(PyExc_TypeError, "required: 'int' or 'list[S]'");
                return -1;
            }
            self->d[i + size] = seginfo::Sconvert_Py_to_C(item);
        }
        for (int i = size - 1; i >= 1; i--) {
            segtree_update(self, i);
        }
    } else {
        PyErr_SetString(PyExc_TypeError, "required: 'int' or 'list[S]'");
        return -1;
    }

    return 0;
}

static PyObject *
segtree_new(PyTypeObject *type, PyObject *args, PyObject *kwargs)
{
    SegTreeObject *self;
    self = (SegTreeObject *)type->tp_alloc(type, 0);
    if (self != NULL) {
        self->_n = 0;
        self->d = std::vector<seginfo::S>();
        self->pyS = Py_None;
        Py_INCREF(Py_None);
    }
    return (PyObject *)self;
}


static void
segtree_dealloc(SegTreeObject *self) {
    Py_XDECREF(self->pyS);
    Py_TYPE(self)->tp_free((PyObject*)self);
}


PyTypeObject SegTreeType = {
    .ob_base = {PyObject_HEAD_INIT(NULL) 0},
    .tp_name = "atcoder.SegTree",
    .tp_basicsize = sizeof(SegTreeObject),
    .tp_itemsize = 0,
    // .tp_itemsize = sizeof(int),
    .tp_dealloc = (destructor)segtree_dealloc,
    .tp_repr = (reprfunc)segtree_repr,
    .tp_as_sequence = &segtree_as_sequence,
    // .tp_getattro = PyObject_GenericGetAttr,
    // .tp_setattro = PyObject_GenericSetAttr,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_doc = segtree_doc,
    .tp_methods = segtree_methods,
    // .tp_members = dsu_members,
    .tp_init = (initproc)segtree_init,
    .tp_new = segtree_new,
    .tp_free = PyObject_Del,
};


} // namespace atcoder_python

#endif  // ACL_PYTHON_SEGTREE